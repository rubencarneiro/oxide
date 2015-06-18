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

// TODO: Remove the use of FrameTreeNode / RenderFrameHostImpl if we get
// another way to map a RenderFrameHost to an individual node in the frame tree
// (which is what WebFrame represents)

#include "oxide_web_frame.h"

#include <map>
#include <utility>

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "content/browser/frame_host/frame_tree.h"
#include "content/browser/frame_host/frame_tree_node.h"
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"

#include "shared/common/oxide_messages.h"
#include "shared/common/oxide_script_message_params.h"

#include "oxide_script_message_request_impl_browser.h"
#include "oxide_web_view.h"

namespace oxide {

namespace {

typedef std::map<int, WebFrame*> FrameMap;
typedef FrameMap::iterator FrameMapIterator;

base::LazyInstance<FrameMap> g_frame_map = LAZY_INSTANCE_INITIALIZER;

}

void WebFrame::WillDestroy() {
  DCHECK(!destroyed_);

  while (GetChildCount() > 0) {
    WebFrame* child = GetChildAt(0);
    child->WillDestroy();
    delete child;
  }

  if (parent_) {
    parent_->RemoveChild(this);
  }

  int id = static_cast<content::RenderFrameHostImpl*>(render_frame_host_)
      ->frame_tree_node()
      ->frame_tree_node_id();
  size_t erased = g_frame_map.Get().erase(id);
  DCHECK_GT(erased, 0U);

  destroyed_ = true;

  for (auto it = current_script_message_requests_.begin();
       it != current_script_message_requests_.end(); ++it) {
    ScriptMessageRequestImplBrowser* request = *it;
    if (!request) {
      continue;
    }

    if (!request->IsWaitingForResponse()) {
      continue;
    }

    scoped_ptr<base::ListValue> wrapped_payload(new base::ListValue());
    request->OnReceiveResponse(
        wrapped_payload.get(),
        ScriptMessageParams::ERROR_HANDLER_DID_NOT_RESPOND);
  }
}

void WebFrame::Delete() {
  delete this;
}

void WebFrame::AddChild(WebFrame* child) {
  child_frames_.push_back(child);
  OnChildAdded(child);
}

void WebFrame::RemoveChild(WebFrame* child) {
  ChildVector::iterator it =
      std::find(child_frames_.begin(), child_frames_.end(), child);
  DCHECK(it != child_frames_.end());
  child_frames_.erase(it);
  OnChildRemoved(child);
}

size_t WebFrame::GetScriptMessageHandlerCount() const {
  return 0;
}

const ScriptMessageHandler* WebFrame::GetScriptMessageHandlerAt(
    size_t index) const {
  return nullptr;
}

void WebFrame::OnChildAdded(WebFrame* child) {}
void WebFrame::OnChildRemoved(WebFrame* child) {}

WebFrame::~WebFrame() {
  CHECK(destroyed_) << "WebFrame deleted without calling WillDestroy()";
}

WebFrame::WebFrame(content::RenderFrameHost* render_frame_host,
                   WebView* view)
    : parent_(nullptr),
      view_(view->AsWeakPtr()),
      render_frame_host_(render_frame_host),
      next_message_serial_(0),
      destroyed_(false),
      weak_factory_(this) {
  int id = static_cast<content::RenderFrameHostImpl*>(render_frame_host)
      ->frame_tree_node()
      ->frame_tree_node_id();
  std::pair<FrameMapIterator, bool> rv =
      g_frame_map.Get().insert(std::make_pair(id, this));
  DCHECK(rv.second);
}

// static
WebFrame* WebFrame::FromFrameTreeNodeID(int frame_tree_node_id) {
  FrameMapIterator it = g_frame_map.Get().find(frame_tree_node_id);
  return it == g_frame_map.Get().end() ? nullptr : it->second;
}

// static
WebFrame* WebFrame::FromRenderFrameHost(
    content::RenderFrameHost* render_frame_host) {
  if (!render_frame_host) {
    return nullptr;
  }

  return FromFrameTreeNodeID(
      static_cast<content::RenderFrameHostImpl*>(render_frame_host)
        ->frame_tree_node()
        ->frame_tree_node_id());
}

// static
void WebFrame::Destroy(WebFrame* frame) {
  frame->WillDestroy();
  frame->Delete();
}

GURL WebFrame::GetURL() const {
  return render_frame_host_->GetLastCommittedURL();
}

void WebFrame::InitParent(WebFrame* parent) {
  DCHECK(!parent_);
  DCHECK_EQ(parent->view(), view());
  parent_ = parent;
  parent_->AddChild(this);
}

void WebFrame::SetRenderFrameHost(
    content::RenderFrameHost* render_frame_host) {
  DCHECK_EQ(static_cast<content::RenderFrameHostImpl*>(render_frame_host)
                ->frame_tree_node()->frame_tree_node_id(),
            static_cast<content::RenderFrameHostImpl*>(render_frame_host_)
                ->frame_tree_node()->frame_tree_node_id());
  render_frame_host_ = render_frame_host;
}

size_t WebFrame::GetChildCount() const {
  return child_frames_.size();
}

WebFrame* WebFrame::GetChildAt(size_t index) const {
  if (index >= child_frames_.size()) {
    return nullptr;
  }

  return child_frames_.at(index);
}

scoped_ptr<ScriptMessageRequestImplBrowser> WebFrame::SendMessage(
    const GURL& context,
    const std::string& msg_id,
    scoped_ptr<base::Value> payload) {
  if (destroyed_) {
    return nullptr;
  }

  scoped_ptr<ScriptMessageRequestImplBrowser> request(
      new ScriptMessageRequestImplBrowser(this,
                                          next_message_serial_++));

  ScriptMessageParams params;
  PopulateScriptMessageParams(request->serial(),
                              true,
                              context,
                              msg_id,
                              payload.Pass(),
                              &params);

  if (!render_frame_host_->Send(new OxideMsg_SendMessage(
          render_frame_host_->GetRoutingID(), params))) {
    return nullptr;
  }

  current_script_message_requests_.push_back(request.get());

  return request.Pass();
}

bool WebFrame::SendMessageNoReply(const GURL& context,
                                  const std::string& msg_id,
                                  scoped_ptr<base::Value> payload) {
  if (destroyed_) {
    return false;
  }

  ScriptMessageParams params;
  PopulateScriptMessageParams(next_message_serial_++,
                              false,
                              context,
                              msg_id,
                              payload.Pass(),
                              &params);

  return render_frame_host_->Send(
      new OxideMsg_SendMessage(render_frame_host_->GetRoutingID(), params));
}

void WebFrame::RemoveScriptMessageRequest(
    ScriptMessageRequestImplBrowser* req) {
  ScriptMessageRequestVector::iterator it =
      std::find(current_script_message_requests_.begin(),
                current_script_message_requests_.end(),
                req);
  DCHECK(it != current_script_message_requests_.end());

  if (!destroyed_) {
    current_script_message_requests_.erase(it);
  } else {
    // Don't mutate the vector if we're in the destructor
    *it = nullptr;
  }
}

void WebFrame::DidCommitNewURL() {}

} // namespace oxide
