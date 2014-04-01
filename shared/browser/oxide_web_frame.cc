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

#include <map>
#include <utility>

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "content/browser/frame_host/frame_tree.h"
#include "content/browser/frame_host/frame_tree_node.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"

#include "oxide_script_message_request_impl_browser.h"
#include "oxide_web_view.h"

namespace oxide {

namespace {
typedef std::map<int64, WebFrame*> FrameMap;
typedef FrameMap::iterator FrameMapIterator;
base::LazyInstance<FrameMap> g_frame_map = LAZY_INSTANCE_INITIALIZER;
}

size_t WebFrame::GetScriptMessageHandlerCount() const {
  return 0;
}

ScriptMessageHandler* WebFrame::GetScriptMessageHandlerAt(size_t index) const {
  return NULL;
}

void WebFrame::AddChild(WebFrame* frame) {
  child_frames_.push_back(frame);
  OnChildAdded(frame);
  view()->FrameAdded(frame);
}

void WebFrame::RemoveChild(WebFrame* frame) {
  for (ChildVector::iterator it = child_frames_.begin();
       it != child_frames_.end(); ++it) {
    WebFrame* f = *it;

    if (f == frame) {
      OnChildRemoved(frame);
      view()->FrameRemoved(frame);
      child_frames_.erase(it);
      return;
    }
  }
}

void WebFrame::AddScriptMessageRequest(ScriptMessageRequestImplBrowser* req) {
  for (ScriptMessageRequestVector::iterator it =
        current_script_message_requests_.begin();
       it != current_script_message_requests_.end(); ++it) {
    if ((*it) == req) {
      return;
    }
  }

  current_script_message_requests_.push_back(req);
}

void WebFrame::RemoveScriptMessageRequest(
    ScriptMessageRequestImplBrowser* req) {
  for (ScriptMessageRequestVector::iterator it =
        current_script_message_requests_.begin();
       it != current_script_message_requests_.end(); ++it) {
    if ((*it) == req) {
      current_script_message_requests_.erase(it);
      return;
    }
  }
}

void WebFrame::OnChildAdded(WebFrame* child) {}
void WebFrame::OnChildRemoved(WebFrame* child) {}
void WebFrame::OnURLChanged() {}

WebFrame::WebFrame(
    content::FrameTreeNode* node,
    WebView* view) :
    frame_tree_node_id_(node->frame_tree_node_id()),
    parent_(NULL),
    view_(view),
    next_message_serial_(0),
    weak_factory_(this),
    destroyed_(false) {
  std::pair<FrameMapIterator, bool> rv =
      g_frame_map.Get().insert(std::make_pair(frame_tree_node_id_,
                                              this));
  CHECK(rv.second);
}

WebFrame::~WebFrame() {
  CHECK(destroyed_) << "WebFrame deleted without calling Destroy()";
}

void WebFrame::Destroy() {
  while (ChildCount() > 0) {
    ChildAt(0)->Destroy();
  }

  while (true) {
    ScriptMessageRequestImplBrowser* request = NULL;
    for (ScriptMessageRequestVector::iterator it =
          current_script_message_requests_.begin();
         it != current_script_message_requests_.end(); ++it) {
      if ((*it)->IsWaitingForResponse()) {
        request = *it;
        break;
      }
    }

    if (!request) {
      break;
    }

    request->OnReceiveResponse(
        "The frame disappeared whilst waiting for a response",
        ScriptMessageRequest::ERROR_INVALID_DESTINATION);
  }

  if (parent_) {
    parent_->RemoveChild(this);
  }

  g_frame_map.Get().erase(frame_tree_node_id_);

  destroyed_ = true;
  delete this;
}

// static
WebFrame* WebFrame::FromFrameTreeNode(content::FrameTreeNode* node) {
  return FromFrameTreeNodeID(node->frame_tree_node_id());
}

// static
WebFrame* WebFrame::FromFrameTreeNodeID(int64 frame_tree_node_id) {
  FrameMapIterator it = g_frame_map.Get().find(frame_tree_node_id);
  return it == g_frame_map.Get().end() ? NULL : it->second;
}

content::FrameTreeNode* WebFrame::GetFrameTreeNode() {
  return view_->GetFrameTree()->FindByID(frame_tree_node_id_);
}

void WebFrame::SetURL(const GURL& url) {
  url_ = url;
  OnURLChanged();
}

void WebFrame::SetParent(WebFrame* parent) {
  DCHECK(!parent_) << "Changing parents is not supported";
  parent_ = parent;
  parent_->AddChild(this);
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

ScriptMessageRequestImplBrowser* WebFrame::SendMessage(
    const GURL& context,
    const std::string& msg_id,
    const std::string& args) {
  ScriptMessageRequestImplBrowser* request =
      new ScriptMessageRequestImplBrowser(this, next_message_serial_++,
                                          context, true, msg_id, args);
  if (!request->SendMessage()) {
    delete request;
    return NULL;
  }

  return request;
}

bool WebFrame::SendMessageNoReply(const GURL& context,
                                  const std::string& msg_id,
                                  const std::string& args) {
  ScriptMessageRequestImplBrowser* request =
      new ScriptMessageRequestImplBrowser(this, next_message_serial_++,
                                          context, false, msg_id, args);
  bool res = request->SendMessage();
  delete request;
  return res;
}

} // namespace oxide
