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

#include "ui/gfx/size.h"

#include "shared/browser/oxide_gpu_utils.h"
#include "qt/core/browser/oxide_qt_render_widget_host_view.h"

namespace oxide {
namespace qt {

unsigned int AcceleratedFrameTextureHandle::GetID() {
  if (!handle_) {
    return 0;
  }

  return handle_->GetTextureID();
}

bool AcceleratedFrameTextureHandle::IsValid() {
  return handle_ != NULL;
}

RenderWidgetHostViewDelegate::RenderWidgetHostViewDelegate() :
    rwhv_(NULL),
    compositor_frame_type_(COMPOSITOR_FRAME_TYPE_INVALID) {}

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

CompositorFrameType
RenderWidgetHostViewDelegate::GetCompositorFrameType() const {
  return compositor_frame_type_;
}

QImage RenderWidgetHostViewDelegate::GetSoftwareFrameImage() {
  DCHECK_EQ(compositor_frame_type_, COMPOSITOR_FRAME_TYPE_SOFTWARE);
  oxide::SoftwareFrameHandle* handle = rwhv_->GetCurrentSoftwareFrameHandle();

  return QImage(static_cast<uchar *>(handle->GetPixels()),
                handle->size_in_pixels().width(),
                handle->size_in_pixels().height(),
                QImage::Format_ARGB32);
}

AcceleratedFrameTextureHandle
RenderWidgetHostViewDelegate::GetAcceleratedFrameTextureHandle() {
  DCHECK_EQ(compositor_frame_type_, COMPOSITOR_FRAME_TYPE_ACCELERATED);
  oxide::AcceleratedFrameHandle* handle = rwhv_->GetCurrentAcceleratedFrameHandle();

  return AcceleratedFrameTextureHandle(
      handle,
      QSize(handle->size_in_pixels().width(),
            handle->size_in_pixels().height()));
}

void RenderWidgetHostViewDelegate::DidComposite() {
  rwhv_->DidCommitCompositorFrame();
}

QVariant RenderWidgetHostViewDelegate::InputMethodQuery(
    Qt::InputMethodQuery query) const {
  return rwhv_->InputMethodQuery(query);
}

RenderWidgetHostViewDelegate::~RenderWidgetHostViewDelegate() {}

void RenderWidgetHostViewDelegate::SetCompositorFrameType(
    CompositorFrameType type) {
  DCHECK(compositor_frame_type_ == COMPOSITOR_FRAME_TYPE_INVALID ||
         compositor_frame_type_ == type);
  compositor_frame_type_ = type;
}

} // namespace qt
} // namespace oxide
