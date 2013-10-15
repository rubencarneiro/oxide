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

#include "oxide_web_frame.h"

#include "base/logging.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"

#include "shared/common/oxide_messages.h"

#include "oxide_outgoing_message_request.h"
#include "oxide_web_frame_tree.h"
#include "oxide_web_view.h"

namespace oxide {

void WebFrame::AddChildFrame(WebFrame* frame) {
  DCHECK_NE(frame->identifier(), -1);

  child_frames_.push_back(frame);
  OnChildAdded(frame);
}

void WebFrame::RemoveChildFrame(WebFrame* frame) {
  for (ChildVector::iterator it = child_frames_.begin();
       it != child_frames_.end(); ++it) {
    WebFrame* f = *it;

    if (f == frame) {
      child_frames_.erase(it);
      OnChildRemoved(frame);
      return;
    }
  }
}

void WebFrame::AddChildrenToQueue(std::queue<WebFrame *>* queue) const {
  for (ChildVector::const_iterator it = child_frames_.begin();
       it != child_frames_.end(); ++it) {
    WebFrame* frame = *it;
    queue->push(frame);
  }
}

void WebFrame::OnChildAdded(WebFrame* child) {}
void WebFrame::OnChildRemoved(WebFrame* child) {}
void WebFrame::OnURLChanged() {}

WebFrame::WebFrame() :
    id_(-1),
    parent_(NULL),
    tree_(NULL),
    next_message_serial_(0),
    destroyed_(false),
    weak_factory_(this) {}

WebFrame::~WebFrame() {
  DCHECK(destroyed_) <<
      "WebFrame's destructor must only be called via DestroyFrame";
}

void WebFrame::DestroyFrame() {
  while (ChildCount() > 0) {
    ChildAt(ChildCount() - 1)->DestroyFrame();
  }

  while (true) {
    OutgoingMessageRequest* request = NULL;
    for (size_t i = 0; i < GetOutgoingMessageRequestCount(); ++i) {
      OutgoingMessageRequest* tmp = GetOutgoingMessageRequestAt(i);
      if (tmp->IsWaiting()) {
        request = tmp;
        break;
      }
    }

    if (!request) {
      break;
    }

    request->SendError(OxideMsg_SendMessage_Error::FRAME_DISAPPEARED,
                       "The frame disappeared whilst waiting for a response");
  }

  if (parent_) {
    parent_->RemoveChildFrame(this);
    parent_ = NULL;
  }

  destroyed_ = true;

  delete this;
}

WebView* WebFrame::GetView() const {
  return tree_->GetView();
}

content::RenderViewHost* WebFrame::GetRenderViewHost() const {
  return tree_->GetRenderViewHost();
}

void WebFrame::SetURL(const GURL& url) {
  url_ = url;
  OnURLChanged();
}

void WebFrame::SetParent(WebFrame* parent) {
  DCHECK(!parent_) << "Changing parents is not supported";
  parent_ = parent;
  parent_->AddChildFrame(this);
}

WebFrame* WebFrame::FindFrameWithID(int64 frame_id) const {
  std::queue<WebFrame *> q;
  q.push(const_cast<WebFrame *>(this));

  while (!q.empty()) {
    WebFrame* f = q.front();
    q.pop();

    if (f->identifier() == frame_id) {
      return f;
    }

    f->AddChildrenToQueue(&q);
  }

  return NULL;
}

size_t WebFrame::ChildCount() const {
  return child_frames_.size();
}

WebFrame* WebFrame::ChildAt(size_t index) const {
  if (index >= child_frames_.size()) {
    return NULL;
  }

  return child_frames_.at(index);
}

bool WebFrame::SendMessage(const std::string& world_id,
                           const std::string& msg_id,
                           const std::string& args,
                           OutgoingMessageRequest* req) {
  DCHECK(req);

  int serial = next_message_serial_++;

  req->set_serial(serial);

  OxideMsg_SendMessage_Params params;
  params.frame_id = identifier();
  params.world_id = world_id;
  params.serial = serial;
  params.type = OxideMsg_SendMessage_Type::Message;
  params.msg_id = msg_id;
  params.args = args;

  return GetRenderViewHost()->Send(new OxideMsg_SendMessage(
      GetRenderViewHost()->GetRoutingID(), params));
}

bool WebFrame::SendMessageNoReply(const std::string& world_id,
                                  const std::string& msg_id,
                                  const std::string& args) {
  OxideMsg_SendMessage_Params params;
  params.frame_id = identifier();
  params.world_id = world_id;
  params.serial = -1;
  params.type = OxideMsg_SendMessage_Type::Message;
  params.msg_id = msg_id;
  params.args = args;

  return GetRenderViewHost()->Send(new OxideMsg_SendMessage(
      GetRenderViewHost()->GetRoutingID(), params));
}

size_t WebFrame::GetMessageHandlerCount() const {
  return 0;
}

MessageHandler* WebFrame::GetMessageHandlerAt(size_t index) const {
  return NULL;
}

size_t WebFrame::GetOutgoingMessageRequestCount() const {
  return 0;
}

OutgoingMessageRequest* WebFrame::GetOutgoingMessageRequestAt(
    size_t index) const {
  return NULL;
}

} // namespace oxide
