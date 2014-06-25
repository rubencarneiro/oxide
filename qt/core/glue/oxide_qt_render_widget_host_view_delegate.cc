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

#include "oxide_qt_render_widget_host_view_delegate.h"

#include "qt/core/browser/oxide_qt_render_widget_host_view.h"

namespace oxide {
namespace qt {

RenderWidgetHostViewDelegate::RenderWidgetHostViewDelegate() :
    rwhv_(NULL) {}

void RenderWidgetHostViewDelegate::HandleFocusEvent(QFocusEvent* event) {
  rwhv_->HandleFocusEvent(event);
}

void RenderWidgetHostViewDelegate::HandleKeyEvent(QKeyEvent* event) {
  rwhv_->HandleKeyEvent(event);
}

void RenderWidgetHostViewDelegate::HandleMouseEvent(QMouseEvent* event) {
  rwhv_->HandleMouseEvent(event);
}

void RenderWidgetHostViewDelegate::HandleWheelEvent(QWheelEvent* event) {
  rwhv_->HandleWheelEvent(event);
}

void RenderWidgetHostViewDelegate::HandleInputMethodEvent(
    QInputMethodEvent* event) {
  rwhv_->HandleInputMethodEvent(event);
}

void RenderWidgetHostViewDelegate::HandleTouchEvent(
    QTouchEvent* event) {
  rwhv_->HandleTouchEvent(event);
}

void RenderWidgetHostViewDelegate::HandleGeometryChanged() {
  rwhv_->HandleGeometryChanged();
}

QVariant RenderWidgetHostViewDelegate::InputMethodQuery(
    Qt::InputMethodQuery query) const {
  return rwhv_->InputMethodQuery(query);
}

RenderWidgetHostViewDelegate::~RenderWidgetHostViewDelegate() {}

} // namespace qt
} // namespace oxide
