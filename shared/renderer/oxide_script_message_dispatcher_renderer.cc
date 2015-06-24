// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "oxide_script_message_dispatcher_renderer.h"

#include <map>
#include <utility>

#include "base/lazy_instance.h"
#include "base/memory/ref_counted.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "ipc/ipc_message.h"
#include "ipc/ipc_message_macros.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "url/gurl.h"

#include "shared/common/oxide_constants.h"
#include "shared/common/oxide_messages.h"
#include "shared/common/oxide_script_message_request.h"

#include "oxide_isolated_world_map.h"
#include "oxide_script_message_handler_renderer.h"
#include "oxide_script_message_impl_renderer.h"
#include "oxide_script_message_manager.h"
#include "oxide_script_message_object_handler.h"
#include "oxide_script_message_request_impl_renderer.h"

namespace oxide {

namespace {
typedef std::map<content::RenderFrame*, ScriptMessageDispatcherRenderer*>
    ScriptMessageDispatcherMap;
base::LazyInstance<ScriptMessageDispatcherMap> g_dispatcher_map =
    LAZY_INSTANCE_INITIALIZER;
}

void ScriptMessageDispatcherRenderer::OnReceiveMessage(
    const IPC::Message& message) {
  OxideHostMsg_SendMessage::Param p;
  if (!OxideHostMsg_SendMessage::Read(&message, &p)) {
    LOG(ERROR) << "Failed to serialize message";
    return;
  }

  ScriptMessageParams params(std::move(base::get<0>(p)));

  bool is_reply = params.type == ScriptMessageParams::TYPE_REPLY;

  linked_ptr<ScriptMessageManager> mm;
  for (ScriptMessageManagerVector::iterator it =
           script_message_managers_.begin();
       it != script_message_managers_.end(); ++it) {
    if ((*it)->GetContextURL() == params.context) {
      mm = *it;
    }
  }

  if (!mm.get()) {
    if (!is_reply) {
      ReturnError(ScriptMessageParams::ERROR_INVALID_CONTEXT,
                  params);
    }
    return;
  }

  if (!is_reply) {
    v8::HandleScope handle_scope(mm->isolate());
    v8::Context::Scope context_scope(mm->GetV8Context());

    v8::Handle<v8::Object> handle(
        mm->script_message_object_handler().NewInstance());
    scoped_refptr<ScriptMessageImplRenderer> message(
        new ScriptMessageImplRenderer(mm.get(),
                                      params.serial,
                                      params.msg_id,
                                      &params.wrapped_payload,
                                      handle));

    ScriptMessageHandlerRenderer* handler =
        mm->GetHandlerForMsgID(message->msg_id());
    if (handler) {
      handler->handler().OnReceiveMessage(message.get());
    } else {
      message->Error(ScriptMessageParams::ERROR_NO_HANDLER);
    }

    return;
  }


  for (ScriptMessageManager::ScriptMessageRequestVector::const_iterator it =
        mm->current_script_message_requests().begin();
       it != mm->current_script_message_requests().end(); ++it) {
    ScriptMessageRequestImplRenderer* request = *it;
    if (request->serial() == params.serial &&
        request->IsWaitingForResponse()) {
      request->OnReceiveResponse(&params.wrapped_payload, params.error);
      return;
    }
  }
}

void ScriptMessageDispatcherRenderer::ReturnError(
    ScriptMessageParams::Error error,
    const ScriptMessageParams& orig) {
  ScriptMessageParams params;
  params.context = orig.context;
  params.serial = orig.serial,
  params.type = ScriptMessageParams::TYPE_REPLY;
  params.error = error;
  params.msg_id = orig.msg_id;

  Send(new OxideHostMsg_SendMessage(routing_id(), params));
}

void ScriptMessageDispatcherRenderer::DidCreateScriptContext(
    v8::Handle<v8::Context> context,
    int extension_id,
    int world_id) {
  script_message_managers_.push_back(
      linked_ptr<ScriptMessageManager>(new ScriptMessageManager(render_frame(),
                                                                context,
                                                                world_id)));
}

void ScriptMessageDispatcherRenderer::WillReleaseScriptContext(
    v8::Handle<v8::Context> context,
    int world_id) {
  v8::HandleScope handle_scope(context->GetIsolate());

  for (ScriptMessageManagerVector::iterator it =
           script_message_managers_.begin();
       it != script_message_managers_.end(); ++it) {
    if ((*it)->GetV8Context() == context) {
      script_message_managers_.erase(it);
      break;
    }
  }
}

bool ScriptMessageDispatcherRenderer::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ScriptMessageDispatcherRenderer, message)
    IPC_MESSAGE_HANDLER_GENERIC(OxideMsg_SendMessage,
                                OnReceiveMessage(message))
    IPC_MESSAGE_UNHANDLED(handled = false)
    (void)param__;
  IPC_END_MESSAGE_MAP()

  return handled;
}

ScriptMessageDispatcherRenderer::ScriptMessageDispatcherRenderer(
    content::RenderFrame* frame) :
    content::RenderFrameObserver(frame) {
  std::pair<ScriptMessageDispatcherMap::iterator, bool> rv =
      g_dispatcher_map.Get().insert(std::make_pair(frame, this));
  CHECK(rv.second);
}

linked_ptr<ScriptMessageManager>
ScriptMessageDispatcherRenderer::ScriptMessageManagerForWorldId(int world_id) {
  linked_ptr<ScriptMessageManager> message_manager;
  for (ScriptMessageManagerVector::iterator it =
           script_message_managers_.begin();
       it != script_message_managers_.end();
       ++it) {
    if ((*it)->frame() == render_frame() && (*it)->world_id() == world_id) {
      message_manager = *it;
    }
  }
  return message_manager;
}

ScriptMessageDispatcherRenderer::~ScriptMessageDispatcherRenderer() {
  // RenderFrameObserver has already cleared it's pointer to our RenderFrame
  for (ScriptMessageDispatcherMap::iterator it = g_dispatcher_map.Get().begin();
       it != g_dispatcher_map.Get().end(); ++it) {
    ScriptMessageDispatcherMap::value_type& v = *it;
    if (v.second == this) {
      g_dispatcher_map.Get().erase(it);
      break;
    }
  }
}

// static
ScriptMessageDispatcherRenderer* ScriptMessageDispatcherRenderer::FromWebFrame(
    blink::WebLocalFrame* frame) {
  content::RenderFrame* rf = content::RenderFrame::FromWebFrame(frame);
  ScriptMessageDispatcherMap::iterator it = g_dispatcher_map.Get().find(rf);
  return it == g_dispatcher_map.Get().end() ? nullptr : it->second;
}

} // namespace oxide
