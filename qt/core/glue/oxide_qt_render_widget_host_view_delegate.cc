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
#include "oxide_qt_render_widget_host_view_delegate_p.h"

#include "base/memory/ref_counted.h"
#include "ui/gfx/size.h"

#include "shared/browser/oxide_gpu_utils.h"
#include "qt/core/browser/oxide_qt_backing_store.h"
#include "qt/core/browser/oxide_qt_render_widget_host_view.h"

namespace oxide {
namespace qt {

TextureHandleImpl::TextureHandleImpl() {}

TextureHandleImpl::~TextureHandleImpl() {}

unsigned int TextureHandleImpl::GetID() const {
  if (!handle_) {
    return 0;
  }

  return handle_->GetID();
}

QSize TextureHandleImpl::GetSize() const {
  if (!handle_) {
    return QSize();
  }

  gfx::Size size(handle_->GetSize());
  return QSize(size.width(), size.height());
}

void TextureHandleImpl::SetHandle(oxide::TextureHandle* handle) {
  handle_ = handle;
}

RenderWidgetHostViewDelegatePrivate::RenderWidgetHostViewDelegatePrivate() :
    rwhv(NULL) {}

// static
RenderWidgetHostViewDelegatePrivate*
RenderWidgetHostViewDelegatePrivate::get(
    RenderWidgetHostViewDelegate* delegate) {
  return delegate->priv.data();
}

RenderWidgetHostViewDelegate::RenderWidgetHostViewDelegate() :
    priv(new RenderWidgetHostViewDelegatePrivate()) {}

void RenderWidgetHostViewDelegate::HandleFocusEvent(QFocusEvent* event) {
  priv->rwhv->HandleFocusEvent(event);
}

void RenderWidgetHostViewDelegate::HandleKeyEvent(QKeyEvent* event) {
  priv->rwhv->HandleKeyEvent(event);
}

void RenderWidgetHostViewDelegate::HandleMouseEvent(QMouseEvent* event) {
  priv->rwhv->HandleMouseEvent(event);
}

void RenderWidgetHostViewDelegate::HandleWheelEvent(QWheelEvent* event) {
  priv->rwhv->HandleWheelEvent(event);
}

void RenderWidgetHostViewDelegate::HandleInputMethodEvent(
    QInputMethodEvent* event) {
  priv->rwhv->HandleInputMethodEvent(event);
}

void RenderWidgetHostViewDelegate::HandleTouchEvent(
    QTouchEvent* event) {
  priv->rwhv->HandleTouchEvent(event);
}

TextureHandle* RenderWidgetHostViewDelegate::GetCurrentTextureHandle() {
  priv->current_texture_handle_.SetHandle(
      priv->rwhv->GetCurrentTextureHandle());
  return &priv->current_texture_handle_;
}

void RenderWidgetHostViewDelegate::DidUpdate(bool skipped) {
  priv->rwhv->DidUpdate(skipped);
}

QVariant RenderWidgetHostViewDelegate::InputMethodQuery(
    Qt::InputMethodQuery query) const {
  return priv->rwhv->InputMethodQuery(query);
}

const QPixmap* RenderWidgetHostViewDelegate::GetBackingStore() {
  return priv->rwhv->GetBackingStore();
}

RenderWidgetHostViewDelegate::~RenderWidgetHostViewDelegate() {}

} // namespace qt
} // namespace oxide
