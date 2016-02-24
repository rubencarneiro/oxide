// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_GLUE_CONTENTS_VIEW_PROXY_H_
#define _OXIDE_QT_CORE_GLUE_CONTENTS_VIEW_PROXY_H_

#include <Qt>
#include <QtGlobal>
#include <QVariant>

typedef void* EGLImageKHR;

QT_BEGIN_NAMESPACE
class QDragEnterEvent;
class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;
class QFocusEvent;
class QHoverEvent;
class QInputMethodEvent;
class QKeyEvent;
class QMouseEvent;
class QRect;
class QTouchEvent;
class QWheelEvent;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class CompositorFrameHandle {
 public:
  virtual ~CompositorFrameHandle() {}

  enum Type {
    TYPE_INVALID,
    TYPE_SOFTWARE,
    TYPE_ACCELERATED,
    TYPE_IMAGE
  };

  virtual Type GetType() = 0;
  virtual const QRect& GetRect() const = 0;

  virtual QImage GetSoftwareFrame() = 0;
  virtual unsigned int GetAcceleratedFrameTexture() = 0;
  virtual EGLImageKHR GetImageFrame() = 0;
};

class ContentsViewProxy {
 public:
  virtual ~ContentsViewProxy() {}

  virtual QSharedPointer<CompositorFrameHandle> compositorFrameHandle() = 0;
  virtual void didCommitCompositorFrame() = 0;

  virtual void wasResized() = 0;
  virtual void visibilityChanged() = 0;
  virtual void screenUpdated() = 0;

  virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const = 0;

  virtual void handleKeyEvent(QKeyEvent* event) = 0;
  virtual void handleInputMethodEvent(QInputMethodEvent* event) = 0;
  virtual void handleFocusEvent(QFocusEvent* event) = 0;
  virtual void handleMouseEvent(QMouseEvent* event) = 0;
  virtual void handleWheelEvent(QWheelEvent* event,
                                const QPoint& window_pos) = 0;
  virtual void handleTouchEvent(QTouchEvent* event) = 0;
  virtual void handleHoverEvent(QHoverEvent* event,
                                const QPoint& window_pos,
                                const QPoint& global_pos) = 0;
  virtual void handleDragEnterEvent(QDragEnterEvent* event) = 0;
  virtual void handleDragMoveEvent(QDragMoveEvent* event) = 0;
  virtual void handleDragLeaveEvent(QDragLeaveEvent* event) = 0;
  virtual void handleDropEvent(QDropEvent* event) = 0;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_CONTENTS_VIEW_PROXY_H_
