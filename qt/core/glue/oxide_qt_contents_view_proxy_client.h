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

#ifndef _OXIDE_QT_CORE_GLUE_CONTENTS_VIEW_PROXY_CLIENT_H_
#define _OXIDE_QT_CORE_GLUE_CONTENTS_VIEW_PROXY_CLIENT_H_

#include <memory>
#include <QRect>
#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QCursor;
class QKeyEvent;
template <typename T> class QList;
class QRectF;
class QScreen;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class ContentsView;
class ContentsViewProxy;
struct MenuItem;
class TouchHandleDrawableProxy;
class WebContextMenuProxy;
class WebContextMenuProxyClient;
class WebPopupMenuProxy;
class WebPopupMenuProxyClient;

class ContentsViewProxyClient {
 public:
  virtual ~ContentsViewProxyClient() {}

  ContentsViewProxy* proxy() const { return proxy_; }

  virtual QScreen* GetScreen() const = 0;

  virtual bool IsVisible() const = 0;

  virtual bool HasFocus() const = 0;

  virtual QRect GetBounds() const = 0;

  virtual void ScheduleUpdate() = 0;
  virtual void EvictCurrentFrame() = 0;

  virtual void UpdateCursor(const QCursor& cursor) = 0;

  virtual void SetInputMethodEnabled(bool enabled) = 0;

  virtual WebContextMenuProxy* CreateWebContextMenu(
      WebContextMenuProxyClient* client) = 0;

  virtual std::unique_ptr<WebPopupMenuProxy> CreateWebPopupMenu(
      const QList<MenuItem>& items,
      bool allow_multiple_selection,
      WebPopupMenuProxyClient* client) = 0;

  virtual TouchHandleDrawableProxy* CreateTouchHandleDrawable() = 0;

  virtual void TouchSelectionChanged(bool active,
                                     const QRectF& bounds,
                                     bool handle_drag_in_progress) = 0;

  virtual void HandleUnhandledKeyboardEvent(QKeyEvent* event) = 0;

 protected:
  ContentsViewProxyClient() : proxy_(nullptr) {}

 private:
  friend class ContentsView;

  ContentsViewProxy* proxy_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_CONTENTS_VIEW_PROXY_CLIENT_H_
