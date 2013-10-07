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

#include "oxide_message_handler.h"
#include "oxide_outgoing_message_request.h"
#include "oxide_web_view.h"

namespace oxide {

void WebFrame::AddChildFrame(WebFrame* frame) {
  child_frames_.push_back(linked_ptr<WebFrame>(frame));
  OnChildAdded(frame);
}

void WebFrame::RemoveChildFrame(WebFrame* frame) {
  for (ChildVector::iterator it = child_frames_.begin();
       it != child_frames_.end(); ++it) {
    linked_ptr<WebFrame> f = *it;

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
    linked_ptr<WebFrame> frame = *it;
    queue->push(frame.get());
  }
}

void WebFrame::OnChildAdded(WebFrame* child) {}
void WebFrame::OnChildRemoved(WebFrame* child) {}
void WebFrame::OnURLChanged() {}

WebFrame::WebFrame(int64 frame_id) :
    id_(frame_id),
    parent_(NULL),
    view_(NULL),
    next_message_serial_(0),
    weak_factory_(this) {}

WebFrame::~WebFrame() {
  MessageDispatcherBrowser::OutgoingMessageRequestVector requests =
      GetOutgoingMessageRequests();
  for (MessageDispatcherBrowser::OutgoingMessageRequestVector::iterator it =
        requests.begin();
       it != requests.end(); ++it) {
    OutgoingMessageRequest* request = *it;

    request->SendError(OxideMsg_SendMessage_Error::FRAME_DISAPPEARED,
                       "The frame disappeared whilst waiting for a response");
  }
}

void WebFrame::DestroyFrame() {
  if (parent_) {
    parent_->RemoveChildFrame(this);
  } else {
    view_->SetRootFrame(NULL);
  }
}

WebView* WebFrame::GetView() const {
  WebFrame* top = const_cast<WebFrame *>(this);
  while (top->parent()) {
    top = top->parent();
  }

  if (top == this) {
    return view_;
  }

  return top->GetView();
}

void WebFrame::SetURL(const GURL& url) {
  url_ = url;
  OnURLChanged();
}

void WebFrame::SetParent(WebView* parent) {
  DCHECK(!view_ && !parent_) << "Changing parents is not supported";
  view_ = parent;
  view_->SetRootFrame(this);
}

void WebFrame::SetParent(WebFrame* parent) {
  DCHECK(!view_ && !parent_) << "Changing parents is not supported";
  parent_ = parent;
  parent_->AddChildFrame(this);
}


WebFrame* WebFrame::FindFrameWithID(int64 frame_id) {
  std::queue<WebFrame *> q;
  q.push(this);

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

  return child_frames_.at(index).get();
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

  content::WebContents* web_contents = GetView()->web_contents();

  return web_contents->Send(new OxideMsg_SendMessage(
      web_contents->GetRenderViewHost()->GetRoutingID(), params));
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

  content::WebContents* web_contents = GetView()->web_contents();

  return web_contents->Send(new OxideMsg_SendMessage(
      web_contents->GetRenderViewHost()->GetRoutingID(), params));
}

MessageDispatcherBrowser::MessageHandlerVector
WebFrame::GetMessageHandlers() const {
  return MessageDispatcherBrowser::MessageHandlerVector();
}

MessageDispatcherBrowser::OutgoingMessageRequestVector
WebFrame::GetOutgoingMessageRequests() const {
  return MessageDispatcherBrowser::OutgoingMessageRequestVector();
}

} // namespace oxide
