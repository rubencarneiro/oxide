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

#include <QImage>

#include "base/logging.h"
#include "ui/gfx/size.h"

#include "shared/browser/compositor/oxide_compositor_frame_handle.h"
#include "qt/core/browser/oxide_qt_render_widget_host_view.h"

namespace oxide {
namespace qt {

class CompositorFrameHandleImpl : public CompositorFrameHandle {
 public:
  CompositorFrameHandleImpl(oxide::CompositorFrameHandle* frame)
      : frame_(frame) {}
  virtual ~CompositorFrameHandleImpl() {}

  CompositorFrameHandle::Type GetType() Q_DECL_FINAL {
    if (frame_->gl_frame_data()) {
      return CompositorFrameHandle::TYPE_ACCELERATED;
    }
    //if (frame_->software_frame_data()) {
    //  return CompositorFrameHandle::TYPE_SOFTWARE;
    //}

    return CompositorFrameHandle::TYPE_INVALID;
  }

  QImage GetSoftwareFrame() Q_DECL_FINAL {
    NOTREACHED();
    return QImage();
  }

  AcceleratedFrameData GetAcceleratedFrame() Q_DECL_FINAL {
    DCHECK(frame_->gl_frame_data());
    return AcceleratedFrameData(
        frame_->gl_frame_data()->texture_id(),
        QSize(frame_->gl_frame_data()->size_in_pixels().width(),
              frame_->gl_frame_data()->size_in_pixels().height()));
  }

 private:
  oxide::CompositorFrameHandle* frame_;
};

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

CompositorFrameHandle*
RenderWidgetHostViewDelegate::GetCompositorFrameHandle() {
  if (!compositor_frame_) {
    compositor_frame_.reset(
        new CompositorFrameHandleImpl(rwhv_->GetCompositorFrameHandle()));
  }

  return compositor_frame_.data();
}

void RenderWidgetHostViewDelegate::DidComposite() {
  rwhv_->DidCommitCompositorFrame();
}

QVariant RenderWidgetHostViewDelegate::InputMethodQuery(
    Qt::InputMethodQuery query) const {
  return rwhv_->InputMethodQuery(query);
}

RenderWidgetHostViewDelegate::~RenderWidgetHostViewDelegate() {}

void RenderWidgetHostViewDelegate::ScheduleUpdate() {
  compositor_frame_.reset();
}

} // namespace qt
} // namespace oxide
