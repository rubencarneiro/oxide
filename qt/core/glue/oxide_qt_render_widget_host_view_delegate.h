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

#ifndef _OXIDE_QT_CORE_GLUE_RENDER_WIDGET_HOST_VIEW_DELEGATE_H_
#define _OXIDE_QT_CORE_GLUE_RENDER_WIDGET_HOST_VIEW_DELEGATE_H_

#include <QImage>
#include <QRect>
#include <QSize>
#include <Qt>
#include <QtGlobal>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QFocusEvent;
class QInputMethodEvent;
class QKeyEvent;
class QMouseEvent;
class QScreen;
class QTouchEvent;
class QWheelEvent;
QT_END_NAMESPACE

namespace content {
class RenderWidgetHost;
}

namespace oxide {

class AcceleratedFrameHandle;

namespace qt {

class RenderWidgetHostView;
class WebViewAdapter;

enum CompositorFrameType {
  COMPOSITOR_FRAME_TYPE_INVALID,
  COMPOSITOR_FRAME_TYPE_SOFTWARE,
  COMPOSITOR_FRAME_TYPE_ACCELERATED
};

class Q_DECL_EXPORT AcceleratedFrameTextureHandle Q_DECL_FINAL {
 public:
  AcceleratedFrameTextureHandle() : handle_(NULL) {}
  AcceleratedFrameTextureHandle(oxide::AcceleratedFrameHandle* handle,
                                const QSize& size)
      : handle_(handle), size_(size) {}
  virtual ~AcceleratedFrameTextureHandle() {}

  unsigned int GetID();
  QSize size() const { return size_; }

  bool IsValid();

 private:
  oxide::AcceleratedFrameHandle* handle_;
  QSize size_;
};

class Q_DECL_EXPORT RenderWidgetHostViewDelegate {
 public:
  virtual ~RenderWidgetHostViewDelegate();

  virtual void Init(WebViewAdapter* view) = 0;

  virtual void Blur() = 0;
  virtual void Focus() = 0;
  virtual bool HasFocus() = 0;

  virtual void Show() = 0;
  virtual void Hide() = 0;
  virtual bool IsShowing() = 0;

  virtual void UpdateCursor(const QCursor& cursor) = 0;

  virtual QRect GetViewBoundsPix() = 0;

  virtual void SetSize(const QSize& size) = 0;

  virtual QScreen* GetScreen() = 0;

  virtual void SetInputMethodEnabled(bool enabled) = 0;

  virtual void ScheduleUpdate() = 0;

  void SetCompositorFrameType(CompositorFrameType type);

 protected:
  RenderWidgetHostViewDelegate();

  void HandleFocusEvent(QFocusEvent* event);
  void HandleKeyEvent(QKeyEvent* event);
  void HandleMouseEvent(QMouseEvent* event);
  void HandleWheelEvent(QWheelEvent* event);
  void HandleInputMethodEvent(QInputMethodEvent* event);
  void HandleTouchEvent(QTouchEvent* event);
  void HandleGeometryChanged();

  CompositorFrameType GetCompositorFrameType() const;
  QImage GetSoftwareFrameImage();
  AcceleratedFrameTextureHandle GetAcceleratedFrameTextureHandle();

  void DidComposite();

  QVariant InputMethodQuery(Qt::InputMethodQuery query) const;

 private:
  friend class RenderWidgetHostView;

  RenderWidgetHostView* rwhv_;
  CompositorFrameType compositor_frame_type_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_RENDER_WIDGET_HOST_VIEW_DELEGATE_H_
