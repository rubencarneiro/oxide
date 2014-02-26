// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#include "oxide_message_dispatcher_renderer.h"

#include <map>
#include <utility>

#include "base/lazy_instance.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_thread.h"
#include "ipc/ipc_message.h"
#include "ipc/ipc_message_macros.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "url/gurl.h"

#include "shared/common/oxide_messages.h"
#include "shared/common/oxide_script_message_request.h"

#include "oxide_isolated_world_map.h"
#include "oxide_v8_message_manager.h"

namespace oxide {

namespace {
typedef std::map<content::RenderFrame*, MessageDispatcherRenderer*> MessageDispatcherMap;
typedef MessageDispatcherMap::iterator MessageDispatcherMapIterator;
base::LazyInstance<MessageDispatcherMap> g_dispatcher_map =
    LAZY_INSTANCE_INITIALIZER;
}

void MessageDispatcherRenderer::OnReceiveMessage(
    const OxideMsg_SendMessage_Params& params) {
  for (MessageManagerVector::iterator it = message_managers_.begin();
       it != message_managers_.end(); ++it) {
    linked_ptr<V8MessageManager> mm = *it;
    if (mm->world_id() == IsolatedWorldMap::IDFromURL(GURL(params.context))) {
      mm->ReceiveMessage(params);
      return;
    }
  }

  if (params.type != OxideMsg_SendMessage_Type::Message) {
    return;
  }

  OxideMsg_SendMessage_Params error_params;
  error_params.context = params.context;
  error_params.serial = params.serial;
  error_params.type = OxideMsg_SendMessage_Type::Reply;
  error_params.error = ScriptMessageRequest::ERROR_INVALID_DESTINATION;
  error_params.msg_id = params.msg_id;
  error_params.payload = std::string(
      "Could not deliver message. The specified world ID does not exist");

  Send(new OxideHostMsg_SendMessage(routing_id(), error_params));
}

bool MessageDispatcherRenderer::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(MessageDispatcherRenderer, message)
    IPC_MESSAGE_HANDLER(OxideMsg_SendMessage, OnReceiveMessage)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

MessageDispatcherRenderer::MessageDispatcherRenderer(
    content::RenderFrame* frame) :
    content::RenderFrameObserver(frame) {
  std::pair<MessageDispatcherMapIterator, bool> rv =
      g_dispatcher_map.Get().insert(std::make_pair(frame, this));
  CHECK(rv.second);
}

MessageDispatcherRenderer::~MessageDispatcherRenderer() {
  // RenderFrameObserver has already cleared it's pointer to our RenderFrame
  for (MessageDispatcherMapIterator it = g_dispatcher_map.Get().begin();
       it != g_dispatcher_map.Get().end(); ++it) {
    MessageDispatcherMap::value_type& v = *it;
    if (v.second == this) {
      g_dispatcher_map.Get().erase(it);
      break;
    }
  }
}

// static
MessageDispatcherRenderer* MessageDispatcherRenderer::FromWebFrame(
    blink::WebFrame* frame) {
  content::RenderFrame* rf = content::RenderFrame::FromWebFrame(frame);
  MessageDispatcherMapIterator it = g_dispatcher_map.Get().find(rf);
  return it == g_dispatcher_map.Get().end() ? NULL : it->second;
}

void MessageDispatcherRenderer::DidCreateScriptContext(
    v8::Handle<v8::Context> context,
    int world_id) {
  if (world_id < 1) {
    return;
  }

  message_managers_.push_back(
      linked_ptr<V8MessageManager>(new V8MessageManager(render_frame(),
                                                        context,
                                                        world_id)));
}

void MessageDispatcherRenderer::WillReleaseScriptContext(
    v8::Handle<v8::Context> context,
    int world_id) {
  if (world_id < 1) {
    return;
  }

  v8::HandleScope handle_scope(v8::Isolate::GetCurrent());

  for (MessageManagerVector::iterator it = message_managers_.begin();
       it != message_managers_.end(); ++it) {
    if ((*it)->v8_context() == context) {
      message_managers_.erase(it);
      break;
    }
  }
}

} // namespace oxide
