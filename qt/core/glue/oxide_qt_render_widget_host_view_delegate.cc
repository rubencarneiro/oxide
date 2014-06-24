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
#include "base/memory/ref_counted.h"
#include "ui/gfx/size.h"

#include "shared/browser/compositor/oxide_compositor_frame_handle.h"
#include "qt/core/browser/oxide_qt_render_widget_host_view.h"

namespace oxide {
namespace qt {

class CompositorFrameHandleImpl : public CompositorFrameHandle {
 public:
  CompositorFrameHandleImpl(oxide::CompositorFrameHandle* frame)
      : frame_(frame) {
    if (frame_) {
      size_ = QSize(frame_->size_in_pixels().width(),
                    frame_->size_in_pixels().height());
    }
  }

  virtual ~CompositorFrameHandleImpl() {}

  CompositorFrameHandle::Type GetType() Q_DECL_FINAL {
    if (!frame_) {
      return CompositorFrameHandle::TYPE_INVALID;
    }
    if (frame_->gl_frame_data()) {
      return CompositorFrameHandle::TYPE_ACCELERATED;
    }
    if (frame_->software_frame_data()) {
      return CompositorFrameHandle::TYPE_SOFTWARE;
    }

    NOTREACHED();
    return CompositorFrameHandle::TYPE_INVALID;
  }

  const QSize& GetSize() const Q_DECL_FINAL {
    return size_;
  }

  QImage GetSoftwareFrame() Q_DECL_FINAL {
    DCHECK_EQ(GetType(), CompositorFrameHandle::TYPE_SOFTWARE);
    return QImage(
        static_cast<uchar *>(frame_->software_frame_data()->pixels()),
        frame_->size_in_pixels().width(),
        frame_->size_in_pixels().height(),
        QImage::Format_ARGB32);
  }

  AcceleratedFrameData GetAcceleratedFrame() Q_DECL_FINAL {
    DCHECK_EQ(GetType(), CompositorFrameHandle::TYPE_ACCELERATED);
    return AcceleratedFrameData(frame_->gl_frame_data()->texture_id());
  }

 private:
  scoped_refptr<oxide::CompositorFrameHandle> frame_;
  QSize size_;
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

QSharedPointer<CompositorFrameHandle>
RenderWidgetHostViewDelegate::GetCompositorFrameHandle() {
  QSharedPointer<CompositorFrameHandle> handle(
      new CompositorFrameHandleImpl(rwhv_->GetCompositorFrameHandle()));
  return handle;
}

void RenderWidgetHostViewDelegate::DidComposite() {
  rwhv_->DidCommitCompositorFrame();
}

QVariant RenderWidgetHostViewDelegate::InputMethodQuery(
    Qt::InputMethodQuery query) const {
  return rwhv_->InputMethodQuery(query);
}

RenderWidgetHostViewDelegate::~RenderWidgetHostViewDelegate() {}

} // namespace qt
} // namespace oxide
