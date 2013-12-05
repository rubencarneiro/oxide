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

#include <QRect>
#include <QSize>
#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QFocusEvent;
class QKeyEvent;
class QMouseEvent;
class QPixmap;
class QScreen;
class QWheelEvent;
QT_END_NAMESPACE

namespace content {
class RenderWidgetHost;
}

namespace oxide {
namespace qt {

class RenderWidgetHostView;

class Q_DECL_EXPORT TextureInfo Q_DECL_FINAL {
 public:
  TextureInfo(unsigned int id, const QSize& size_in_pixels);
  ~TextureInfo();

  unsigned int id() const { return id_; }
  QSize size_in_pixels() const { return size_in_pixels_; }

 private:
  unsigned int id_;
  QSize size_in_pixels_;
};

class Q_DECL_EXPORT RenderWidgetHostViewDelegate {
 public:
  virtual ~RenderWidgetHostViewDelegate();

  virtual void Blur() = 0;
  virtual void Focus() = 0;
  virtual bool HasFocus() = 0;

  virtual void Show() = 0;
  virtual void Hide() = 0;
  virtual bool IsShowing() = 0;

  virtual QRect GetViewBounds() = 0;
  virtual QRect GetBoundsInRootWindow() = 0;

  virtual void SetSize(const QSize& size) = 0;

  virtual QScreen* GetScreen() = 0;

  const QPixmap* GetBackingStore();

 protected:
  RenderWidgetHostViewDelegate();

  void ForwardFocusEvent(QFocusEvent* event);
  void ForwardKeyEvent(QKeyEvent* event);
  void ForwardMouseEvent(QMouseEvent* event);
  void ForwardWheelEvent(QWheelEvent* event);

  TextureInfo GetFrontbufferTextureInfo();
  void DidComposite(bool skipped);

 private:
  friend class RenderWidgetHostView;

  virtual void SchedulePaint(const QRect& rect) = 0;
  virtual void ScheduleComposite() = 0;

  RenderWidgetHostView* GetRenderWidgetHostView();
  void SetRenderWidgetHostView(RenderWidgetHostView* rwhv);

  RenderWidgetHostView* rwhv_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_RENDER_WIDGET_HOST_VIEW_DELEGATE_H_
