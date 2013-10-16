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

#include "oxide_web_frame_tree.h"

#include <map>

#include "base/lazy_instance.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "ipc/ipc_message_macros.h"

#include "shared/common/oxide_messages.h"

#include "oxide_web_frame.h"
#include "oxide_web_view.h"

namespace oxide {

namespace {
typedef std::pair<int32, int32> WebFrameTreeID;
typedef std::map<WebFrameTreeID, WebFrameTree*> WebFrameTreeMap;
base::LazyInstance<WebFrameTreeMap> g_web_frame_tree_map = LAZY_INSTANCE_INITIALIZER;
}

void WebFrameTree::OnFrameCreated(int64 parent_frame_id, int64 frame_id) {
  WebFrame* parent = GetRootFrame()->FindFrameWithID(parent_frame_id);
  if (!parent) {
    LOG(ERROR) << "Got FrameCreated with non-existant parent!";
    render_view_host()->GetProcess()->ReceivedBadMessage();
    return;
  }

  WebFrame* frame = CreateFrame();
  frame->set_identifier(frame_id);
  frame->set_tree(this);
  frame->SetParent(parent);
}

void WebFrameTree::OnFrameDetached(int64 frame_id) {
  WebFrame* frame = GetRootFrame()->FindFrameWithID(frame_id);
  if (!frame) {
    LOG(ERROR) << "Got FrameDetached for non-existant frame!";
    render_view_host()->GetProcess()->ReceivedBadMessage();
    return;
  }

  frame->DestroyFrame();
}

WebFrameTree::WebFrameTree(content::RenderViewHost* rvh) :
    content::RenderViewHostObserver(rvh),
    root_(NULL) {
  g_web_frame_tree_map.Get().insert(std::make_pair(
      WebFrameTreeID(render_view_host()->GetProcess()->GetID(), routing_id()),
      this));
}

WebFrameTree::~WebFrameTree() {
  if (root_) {
    root_->DestroyFrame();
    root_ = NULL;
  }
}

// static
WebFrameTree* WebFrameTree::FromRenderViewHost(content::RenderViewHost* rvh) {
  WebFrameTreeMap* map = g_web_frame_tree_map.Pointer();
  WebFrameTreeMap::iterator it = map->find(
      WebFrameTreeID(rvh->GetProcess()->GetID(), rvh->GetRoutingID()));
  return it != map->end() ? it->second : NULL;

}

WebFrame* WebFrameTree::GetRootFrame() {
  if (!root_) {
    root_ = CreateFrame();
    root_->set_tree(this);

    GetView()->RootFrameCreated(root_);
  }

  return root_;
}

WebView* WebFrameTree::GetView() const {
  return WebView::FromRenderViewHost(render_view_host());
}

content::RenderViewHost* WebFrameTree::GetRenderViewHost() const {
  return render_view_host();
}

void WebFrameTree::RenderViewHostDestroyed(content::RenderViewHost* rvh) {
  g_web_frame_tree_map.Get().erase(
      WebFrameTreeID(rvh->GetProcess()->GetID(), routing_id()));
  delete this;
}

bool WebFrameTree::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  bool msg_is_good = true;
  IPC_BEGIN_MESSAGE_MAP_EX(WebFrameTree, message, msg_is_good)
    IPC_MESSAGE_HANDLER(OxideHostMsg_FrameCreated, OnFrameCreated)
    IPC_MESSAGE_HANDLER(OxideHostMsg_FrameDetached, OnFrameDetached)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP_EX()

  if (!msg_is_good) {
    // Die, naughty renderer
    render_view_host()->GetProcess()->ReceivedBadMessage();
  }

  return handled;
}

} // namespace oxide
