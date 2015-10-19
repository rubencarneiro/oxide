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

#include "oxide_web_frame.h"

#include <map>
#include <utility>

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"

#include "shared/common/oxide_messages.h"
#include "shared/common/oxide_script_message_params.h"

#include "oxide_render_frame_host_id.h"
#include "oxide_script_message_request_impl_browser.h"
#include "oxide_web_frame_tree.h"
#include "oxide_web_view.h"

namespace oxide {

namespace {

typedef std::map<RenderFrameHostID, WebFrame*> WebFrameMap;

base::LazyInstance<WebFrameMap> g_frame_map = LAZY_INSTANCE_INITIALIZER;

void AddMappingForRenderFrameHost(content::RenderFrameHost* host,
                                  WebFrame* frame) {
  RenderFrameHostID id = RenderFrameHostID::FromHost(host);
  DCHECK(g_frame_map.Get().find(id) == g_frame_map.Get().end());
  auto rv = g_frame_map.Get().insert(std::make_pair(id, frame));
  DCHECK(rv.second);
}

void RemoveMappingForRenderFrameHost(content::RenderFrameHost* host) {
  size_t removed = g_frame_map.Get().erase(RenderFrameHostID::FromHost(host));
  DCHECK_EQ(removed, 1U);
}

}

size_t WebFrame::GetScriptMessageHandlerCount() const {
  if (!script_message_target_delegate_) {
    return 0;
  }

  return script_message_target_delegate_->GetScriptMessageHandlerCount();
}

const ScriptMessageHandler* WebFrame::GetScriptMessageHandlerAt(
    size_t index) const {
  if (!script_message_target_delegate_) {
    return nullptr;
  }

  return script_message_target_delegate_->GetScriptMessageHandlerAt(index);
}

WebFrame::WebFrame(WebFrameTree* tree,
                   content::RenderFrameHost* render_frame_host)
    : frame_tree_(tree),
      render_frame_host_(render_frame_host),
      parent_(nullptr),
      next_message_serial_(0),
      destroying_(false),
      script_message_target_delegate_(nullptr),
      weak_factory_(this) {
  AddMappingForRenderFrameHost(render_frame_host, this);
}

WebFrame::~WebFrame() {
  destroying_ = true;

  while (child_frames_.size() > 0) {
    WebFrame* child = child_frames_.back();
    // Remove |child| from our list of children before deleting it, as deleting
    // it causes WebFrameTreeObserver::FrameDeleted to fire. This can result in
    // re-entry via GetChildFrames which would otherwise return an
    // invalid pointer
    child_frames_.pop_back();
    delete child;
  }

  RemoveMappingForRenderFrameHost(render_frame_host_);
  frame_tree_->WebFrameRemoved(this);

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

// static
WebFrame* WebFrame::FromRenderFrameHost(
    content::RenderFrameHost* render_frame_host) {
  if (!render_frame_host) {
    return nullptr;
  }

  auto it = g_frame_map.Get().find(
      RenderFrameHostID::FromHost(render_frame_host));
  return it == g_frame_map.Get().end() ? nullptr : it->second;
}

GURL WebFrame::GetURL() const {
  return render_frame_host_->GetLastCommittedURL();
}

void WebFrame::AddChild(scoped_ptr<WebFrame> child) {
  DCHECK(!child->parent_);
  child->parent_ = this;
  child_frames_.push_back(child.release());
}

void WebFrame::RemoveChild(WebFrame* child) {
  DCHECK_EQ(child->parent_, this);

  auto it = std::find(child_frames_.begin(), child_frames_.end(), child);
  DCHECK(it != child_frames_.end());

  // Remove |child| from our list of children before deleting it, as deleting
  // it causes WebFrameTreeObserver::FrameDeleted to fire. This can result in
  // re-entry via GetChildFrames which would otherwise return an
  // invalid pointer
  child_frames_.erase(it);
  delete child;
}

WebView* WebFrame::GetView() const {
  return WebView::FromRenderFrameHost(render_frame_host_);
}

void WebFrame::RenderFrameHostChanged(
    content::RenderFrameHost* render_frame_host) {
  RemoveMappingForRenderFrameHost(render_frame_host_);
  render_frame_host_ = render_frame_host;
  AddMappingForRenderFrameHost(render_frame_host_, this);
}

const std::vector<WebFrame*>& WebFrame::GetChildFrames() const {
  return child_frames_;
}

scoped_ptr<ScriptMessageRequestImplBrowser> WebFrame::SendMessage(
    const GURL& context,
    const std::string& msg_id,
    scoped_ptr<base::Value> payload) {
  if (destroying_) {
    return nullptr;
  }

  scoped_ptr<ScriptMessageRequestImplBrowser> request(
      new ScriptMessageRequestImplBrowser(this,
                                          next_message_serial_++));

  ScriptMessageParams params;
  PopulateScriptMessageParams(request->serial(),
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
  if (destroying_) {
    return false;
  }

  ScriptMessageParams params;
  PopulateScriptMessageParams(ScriptMessageParams::kInvalidSerial,
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

  if (!destroying_) {
    current_script_message_requests_.erase(it);
  } else {
    // Don't mutate the vector if we're in the destructor
    *it = nullptr;
  }
}

} // namespace oxide
