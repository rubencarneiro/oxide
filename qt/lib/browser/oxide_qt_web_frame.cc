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

#include "oxide_qt_web_frame.h"

#include "qt/lib/public/oxide_q_web_frame.h"

namespace oxide {
namespace qt {

void WebFrame::OnParentChanged() {
  q_web_frame_->parentFrameChanged();
}

void WebFrame::OnChildAdded(oxide::WebFrame* child) {
  OxideQWebFrame* qchild = child ?
      static_cast<WebFrame *>(child)->q_web_frame() : NULL;
  q_web_frame_->childFrameChanged(OxideQWebFrame::ChildAdded, qchild);
}

void WebFrame::OnChildRemoved(oxide::WebFrame* child) {
  OxideQWebFrame* qchild = child ?
      static_cast<WebFrame *>(child)->q_web_frame() : NULL;
  q_web_frame_->childFrameChanged(OxideQWebFrame::ChildRemoved, qchild);
}

void WebFrame::OnURLChanged() {
  q_web_frame_->urlChanged();
}

WebFrame::WebFrame(int64 frame_id) :
    oxide::WebFrame(frame_id),
    q_web_frame_(new OxideQWebFrame(this)) {}

WebFrame::~WebFrame() {}

} // namespace qt
} // namespace oxide
