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

#ifndef _OXIDE_QQUICK_LEGACY_CONTENTS_VIEW_H_
#define _OXIDE_QQUICK_LEGACY_CONTENTS_VIEW_H_

#include <memory>

#include <QObject>
#include <QPointer>
#include <QQmlComponent>

#include "qt/quick/contents_view.h"

class OxideQQuickTouchSelectionController;

namespace oxide {
namespace qquick {

class LegacyContentsView : public ContentsView {
  Q_OBJECT

  Q_DISABLE_COPY(LegacyContentsView)

 public:
  LegacyContentsView(QQuickItem* item);
  ~LegacyContentsView() override;

  QQmlComponent* popup_menu() const { return popup_menu_; }
  void set_popup_menu(QQmlComponent* popup_menu) {
    popup_menu_ = popup_menu;
  }

  OxideQQuickTouchSelectionController* touch_selection_controller() const {
    return touch_selection_controller_.get();
  }

 private:
  // qt::ContentsViewClient implementation
  std::unique_ptr<qt::WebPopupMenu> CreateWebPopupMenu(
      const std::vector<qt::MenuItem>& items,
      bool allow_multiple_selection,
      const QRect& bounds,
      qt::WebPopupMenuClient* client) override;
  std::unique_ptr<qt::TouchHandleDrawable> CreateTouchHandleDrawable() override;
  qt::LegacyTouchEditingClient* GetLegacyTouchEditingClient() override;

  std::unique_ptr<OxideQQuickTouchSelectionController> touch_selection_controller_;

  QPointer<QQmlComponent> popup_menu_;
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QQUICK_LEGACY_CONTENTS_VIEW_H_
