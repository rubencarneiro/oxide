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

#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "third_party/WebKit/public/web/WebFrame.h"

#include "shared/common/oxide_content_client.h"
#include "shared/common/oxide_messages.h"

#include "oxide_content_renderer_client.h"
#include "oxide_isolated_world_map.h"
#include "oxide_v8_message_manager.h"

namespace oxide {

void MessageDispatcherRenderer::EndPoint::OnReceiveMessage(
    const OxideMsg_SendMessage_Params& params) {
  ContentClient::GetInstance()->renderer()->message_dispatcher()->
      OnReceiveMessage(render_view(), params);
}

MessageDispatcherRenderer::EndPoint::EndPoint(content::RenderView* render_view) :
    content::RenderViewObserver(render_view) {}

bool MessageDispatcherRenderer::EndPoint::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(EndPoint, message)
    IPC_MESSAGE_HANDLER(OxideMsg_SendMessage, OnReceiveMessage)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void MessageDispatcherRenderer::OnReceiveMessage(
    content::RenderView* render_view,
    const OxideMsg_SendMessage_Params& params) {
  for (MessageManagerVector::iterator it = message_managers_.begin();
       it != message_managers_.end(); ++it) {
    linked_ptr<V8MessageManager> mm = *it;
    if (mm->frame_id() == params.frame_id &&
        mm->world_id() == IsolatedWorldMap::NameToID(params.world_id) &&
        mm->render_view() == render_view) {
      mm->ReceiveMessage(params);
      return;
    }
  }

  if (params.type != OxideMsg_SendMessage_Type::Message) {
    return;
  }

  OxideMsg_SendMessage_Params error_params;
  error_params.frame_id = params.frame_id;
  error_params.world_id = params.world_id;
  error_params.serial = params.serial;
  error_params.type = OxideMsg_SendMessage_Type::Reply;
  error_params.error = OxideMsg_SendMessage_Error::INVALID_DESTINATION;
  error_params.msg_id = params.msg_id;
  error_params.args = std::string("Invalid frame or world ID");

  content::RenderThread::Get()->Send(
      new OxideHostMsg_SendMessage(render_view->GetRoutingID(),
                                   error_params));
}

MessageDispatcherRenderer::MessageDispatcherRenderer() {}

MessageDispatcherRenderer::~MessageDispatcherRenderer() {}

void MessageDispatcherRenderer::DidCreateScriptContext(
    WebKit::WebFrame* frame,
    v8::Handle<v8::Context> context,
    int world_id) {
  if (world_id < 1) {
    return;
  }

  message_managers_.push_back(
      linked_ptr<V8MessageManager>(new V8MessageManager(frame,
                                                        context,
                                                        world_id)));
}

void MessageDispatcherRenderer::WillReleaseScriptContext(
    WebKit::WebFrame* frame,
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
