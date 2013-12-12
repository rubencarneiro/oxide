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

#include "qt/core/browser/oxide_qt_backing_store.h"
#include "qt/core/browser/oxide_qt_render_widget_host_view.h"

namespace oxide {
namespace qt {

TextureInfo::TextureInfo(unsigned int id, const QSize& size_in_pixels) :
    id_(id),
    size_in_pixels_(size_in_pixels) {}

TextureInfo::~TextureInfo() {}

RenderWidgetHostView* RenderWidgetHostViewDelegate::GetRenderWidgetHostView() {
  return rwhv_;
}

void RenderWidgetHostViewDelegate::SetRenderWidgetHostView(
    RenderWidgetHostView* rwhv) {
  rwhv_ = rwhv;
}

RenderWidgetHostViewDelegate::RenderWidgetHostViewDelegate() {}

void RenderWidgetHostViewDelegate::ForwardFocusEvent(QFocusEvent* event) {
  GetRenderWidgetHostView()->ForwardFocusEvent(event);
}

void RenderWidgetHostViewDelegate::ForwardKeyEvent(QKeyEvent* event) {
  GetRenderWidgetHostView()->ForwardKeyEvent(event);
}

void RenderWidgetHostViewDelegate::ForwardMouseEvent(QMouseEvent* event) {
  GetRenderWidgetHostView()->ForwardMouseEvent(event);
}

void RenderWidgetHostViewDelegate::ForwardWheelEvent(QWheelEvent* event) {
  GetRenderWidgetHostView()->ForwardWheelEvent(event);
}

TextureInfo RenderWidgetHostViewDelegate::GetFrontbufferTextureInfo() {
  oxide::TextureInfo tex = GetRenderWidgetHostView()->GetFrontbufferTextureInfo();
  return TextureInfo(tex.id(),
                     QSize(tex.size_in_pixels().width(),
                           tex.size_in_pixels().height()));
}

void RenderWidgetHostViewDelegate::DidUpdate(bool skipped) {
  GetRenderWidgetHostView()->DidUpdate(skipped);
}

RenderWidgetHostViewDelegate::~RenderWidgetHostViewDelegate() {}

const QPixmap* RenderWidgetHostViewDelegate::GetBackingStore() {
  return GetRenderWidgetHostView()->GetBackingStore();
}

} // namespace qt
} // namespace oxide
