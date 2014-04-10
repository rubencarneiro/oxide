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

#include "base/memory/ref_counted.h"
#include "ui/gfx/size.h"

#include "shared/browser/oxide_gpu_utils.h"
#include "qt/core/browser/oxide_qt_backing_store.h"
#include "qt/core/browser/oxide_qt_render_widget_host_view.h"

namespace oxide {
namespace qt {

namespace {

class TextureHandleImpl FINAL : public TextureHandle {
 public:
  TextureHandleImpl() {}
  ~TextureHandleImpl() {}

  unsigned int GetID() const FINAL;
  QSize GetSize() const FINAL;

  void SetHandle(oxide::TextureHandle* handle);

 private:
  scoped_refptr<oxide::TextureHandle> handle_;
};

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

}

RenderWidgetHostViewDelegate::RenderWidgetHostViewDelegate() :
    rwhv_(NULL),
    texture_handle_(new TextureHandleImpl()),
    backing_store_(NULL) {}

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

void RenderWidgetHostViewDelegate::UpdateTextureHandle() {
  static_cast<TextureHandleImpl *>(texture_handle_.data())->SetHandle(
      rwhv_->GetCurrentTextureHandle());
}

void RenderWidgetHostViewDelegate::DidComposite(bool skipped) {
  rwhv_->DidComposite(skipped);
}

QVariant RenderWidgetHostViewDelegate::InputMethodQuery(
    Qt::InputMethodQuery query) const {
  return rwhv_->InputMethodQuery(query);
}

const QPixmap* RenderWidgetHostViewDelegate::GetBackingStore() {
  const QPixmap* backing_store = backing_store_;
  backing_store_ = NULL;
  return backing_store;
}

void RenderWidgetHostViewDelegate::UpdateBackingStore() {
  backing_store_ = rwhv_->GetBackingStore();
}

RenderWidgetHostViewDelegate::~RenderWidgetHostViewDelegate() {}

} // namespace qt
} // namespace oxide
