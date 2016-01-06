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

#include "oxide_web_frame_tree_observer.h"

#include "oxide_web_frame_tree.h"

namespace oxide {

void WebFrameTreeObserver::OnFrameTreeDestruction() {
  frame_tree_ = nullptr;
  FrameTreeDestroyed();
}

WebFrameTreeObserver::WebFrameTreeObserver()
    : frame_tree_(nullptr) {}

WebFrameTreeObserver::WebFrameTreeObserver(WebFrameTree* tree)
    : frame_tree_(tree) {
  if (tree) {
    tree->AddObserver(this);
  }
}

void WebFrameTreeObserver::Observe(WebFrameTree* tree) {
  if (tree == frame_tree_) {
    return;
  }

  if (frame_tree_) {
    frame_tree_->RemoveObserver(this);
  }
  frame_tree_ = tree;
  if (frame_tree_) {
    frame_tree_->AddObserver(this);
  }
}

WebFrameTreeObserver::~WebFrameTreeObserver() {
  if (frame_tree_) {
    frame_tree_->RemoveObserver(this);
  }
}

} // namespace oxide
