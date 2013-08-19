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

namespace oxide {

void WebFrame::AddChildrenToQueue(std::queue<WebFrame *>* queue) const {
  for (ChildMap::const_iterator it = child_frames_.begin();
       it != child_frames_.end(); ++it) {
    queue->push(it->second.get());
  }
}

WebFrame* WebFrame::GetChildFrameWithID(int64 frame_id) const {
  ChildMap::const_iterator it = child_frames_.find(frame_id);
  if (it != child_frames_.end()) {
    return it->second.get();
  }

  return NULL;
}

void WebFrame::SetParent(WebFrame* parent) {
  parent_ = parent;
  OnParentChanged();
}

void WebFrame::OnParentChanged() {}
void WebFrame::OnChildAdded(WebFrame* child) {}
void WebFrame::OnChildRemoved(WebFrame* child) {}
void WebFrame::OnURLChanged() {}

WebFrame::WebFrame(int64 frame_id) :
  id_(frame_id),
  parent_(NULL) {}

WebFrame::~WebFrame() {}

void WebFrame::AddChildFrame(WebFrame* frame) {
  CHECK(!frame->parent());

  frame->SetParent(this);

  child_frames_[frame->identifier()] = linked_ptr<WebFrame>(frame);
  OnChildAdded(frame);
}

void WebFrame::RemoveChildFrameWithID(int64 frame_id) {
  ChildMap::iterator it = child_frames_.find(frame_id);
  if (it == child_frames_.end()) {
    return;
  }

  linked_ptr<WebFrame> child(it->second);

  child_frames_.erase(frame_id);
  OnChildRemoved(child.get());

  child->SetParent(NULL);
}

void WebFrame::SetURL(const GURL& url) {
  url_ = url;
  OnURLChanged();
}

WebFrame* WebFrame::FindFrameWithIDInSubtree(int64 frame_id) {
  if (identifier() == frame_id) {
    return this;
  }

  std::queue<WebFrame *> q;
  q.push(this);

  while (!q.empty()) {
    WebFrame* f = q.front();
    q.pop();

    WebFrame* rv = f->GetChildFrameWithID(frame_id);
    if (rv) return rv;

    f->AddChildrenToQueue(&q);
  }

  return NULL;
}

std::vector<WebFrame *> WebFrame::GetChildFrames() const {
  std::vector<WebFrame *> frames;

  for (ChildMap::const_iterator it = child_frames_.begin();
       it != child_frames_.end(); ++ it) {
    frames.push_back(it->second.get());
  }

  return frames;
}

} // namespace oxide
