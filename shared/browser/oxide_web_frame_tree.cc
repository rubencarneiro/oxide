// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#include <queue>

#include "base/logging.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"

#include "oxide_web_frame.h"
#include "oxide_web_frame_tree_observer.h"

namespace oxide {

DEFINE_WEB_CONTENTS_USER_DATA_KEY(WebFrameTree);

WebFrameTree::WebFrameTree(content::WebContents* contents)
    : content::WebContentsObserver(contents),
      root_frame_(new WebFrame(this, contents->GetMainFrame())) {}

void WebFrameTree::WebFrameRemoved(WebFrame* frame) {
  FOR_EACH_OBSERVER(WebFrameTreeObserver, observers_, FrameDeleted(frame));
}

void WebFrameTree::AddObserver(WebFrameTreeObserver* observer) {
  observers_.AddObserver(observer);
}

void WebFrameTree::RemoveObserver(WebFrameTreeObserver* observer) {
  observers_.RemoveObserver(observer);
}

void WebFrameTree::RenderFrameCreated(
    content::RenderFrameHost* render_frame_host) {
  if (!render_frame_host->GetParent()) {
    // The root frame is handled in the constructor
    return;
  }

  DCHECK(!WebFrame::FromRenderFrameHost(render_frame_host));

  if (!content::WebContents::FromRenderFrameHost(render_frame_host)) {
    // This is from an interstitial
    return;
  }

  if (render_frame_host->IsCrossProcessSubframe()) {
    // We should already have a WebFrame for this node
    return;
  }

  WebFrame* parent =
      WebFrame::FromRenderFrameHost(render_frame_host->GetParent());
  DCHECK(parent);

  WebFrame* frame = new WebFrame(this, render_frame_host);
  parent->AddChild(make_scoped_ptr(frame));
  
  FOR_EACH_OBSERVER(WebFrameTreeObserver, observers_, FrameCreated(frame));
}

void WebFrameTree::RenderFrameHostChanged(
    content::RenderFrameHost* old_host,
    content::RenderFrameHost* new_host) {
  DCHECK(!WebFrame::FromRenderFrameHost(new_host));

  if (!old_host) {
    DCHECK(!new_host->IsCrossProcessSubframe());
    // This is a new subframe. We delay creating the WebFrame and notifying the
    // client until the corresponding FrameTreeNode has a parent set. We know
    // that Chromium will call in to RenderFrameCreated after this
    return;
  }

  if (!content::WebContents::FromRenderFrameHost(old_host)) {
    // This is from an interstitial
    return;
  }

  WebFrame* frame = WebFrame::FromRenderFrameHost(old_host);
  DCHECK(frame);

  frame->RenderFrameHostChanged(new_host);
}

void WebFrameTree::FrameDeleted(content::RenderFrameHost* render_frame_host) {
  WebFrame* frame = WebFrame::FromRenderFrameHost(render_frame_host);
  if (!frame) {
    // When a frame is detached, we get notified before any of its children
    // are detached. If we hit this case, it means that this is a child of a
    // frame that's being detached, and we've already deleted the corresponding
    // WebFrame
    return;
  }

  if (!frame->parent()) {
    DCHECK_EQ(frame, root_frame_.get());
    root_frame_.reset();
  } else {
    DCHECK_NE(frame, root_frame_.get());
    frame->parent()->RemoveChild(frame);
  }
}

void WebFrameTree::DidCommitProvisionalLoadForFrame(
    content::RenderFrameHost* render_frame_host,
    const GURL& url,
    ui::PageTransition transition_type) {
  WebFrame* frame = WebFrame::FromRenderFrameHost(render_frame_host);
  if (!frame) {
    return;
  }

  FOR_EACH_OBSERVER(
      WebFrameTreeObserver, observers_, LoadCommittedInFrame(frame));
}

WebFrameTree::~WebFrameTree() {
  FOR_EACH_OBSERVER(
      WebFrameTreeObserver, observers_, OnFrameTreeDestruction());
}

void WebFrameTree::ForEachFrame(const ForEachFrameCallback& callback) {
  if (!root_frame_.get()) {
    return;
  }

  std::queue<WebFrame*> queue;
  queue.push(root_frame_.get());

  while (!queue.empty()) {
    WebFrame* frame = queue.front();
    queue.pop();

    if (!callback.Run(frame)) {
      return;
    }

    for (auto child : frame->GetChildFrames()) {
      queue.push(child);
    }
  }
}

} // namespace oxide
