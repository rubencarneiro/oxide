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

#ifndef _OXIDE_QT_CORE_GLUE_CONTENTS_VIEW_CLIENT_H_
#define _OXIDE_QT_CORE_GLUE_CONTENTS_VIEW_CLIENT_H_

#include <memory>
#include <vector>

#include <QRect>
#include <QtGlobal>

#include "qt/core/api/oxideqglobal.h"
#include "qt/core/glue/menu_item.h"
#include "qt/core/glue/contents_view.h"

QT_BEGIN_NAMESPACE
class QCursor;
class QKeyEvent;
class QWindow;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class ContentsView;
class ContentsViewImpl;
class LegacyTouchEditingClient;
struct MenuItem;
class TouchHandleDrawable;
class WebPopupMenu;
class WebPopupMenuClient;

class OXIDE_QTCORE_EXPORT ContentsViewClient {
 public:
  virtual ~ContentsViewClient() = default;

  ContentsView* view() const { return view_; }

  virtual QWindow* GetWindow() const = 0;

  virtual bool IsVisible() const = 0;

  virtual bool HasFocus() const = 0;

  virtual QRect GetBounds() const = 0;

  virtual void ScheduleUpdate() = 0;
  virtual void EvictCurrentFrame() = 0;

  virtual void UpdateCursor(const QCursor& cursor) = 0;

  virtual void SetInputMethodEnabled(bool enabled) = 0;

  virtual std::unique_ptr<WebPopupMenu> CreateWebPopupMenu(
      const std::vector<MenuItem>& items,
      bool allow_multiple_selection,
      const QRect& bounds,
      WebPopupMenuClient* client) = 0;

  virtual std::unique_ptr<TouchHandleDrawable> CreateTouchHandleDrawable() = 0;

  virtual LegacyTouchEditingClient* GetLegacyTouchEditingClient() {
    return nullptr;
  }

  virtual void HandleUnhandledKeyboardEvent(QKeyEvent* event) = 0;

 protected:
  ContentsViewClient() = default;

 private:
  friend class ContentsViewImpl;

  ContentsView* view_ = nullptr;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_CONTENTS_VIEW_CLIENT_H_
