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

#ifndef _OXIDE_QT_UITK_LIB_TOUCH_EDITING_MENU_H_
#define _OXIDE_QT_UITK_LIB_TOUCH_EDITING_MENU_H_

#include <memory>
#include <QPointer>
#include <QtGlobal>
#include <QObject>

#include "qt/core/glue/edit_capability_flags.h"
#include "qt/core/glue/touch_editing_menu.h"

QT_BEGIN_NAMESPACE
class QQuickItem;
QT_END_NAMESPACE

namespace oxide {

namespace qt {
class TouchEditingMenuClient;
}

namespace uitk {

class TouchEditingMenu : public QObject,
                         public qt::TouchEditingMenu {
  Q_OBJECT

  Q_DISABLE_COPY(TouchEditingMenu)

 public:
  static std::unique_ptr<TouchEditingMenu> Create(
      QQuickItem* parent,
      qt::EditCapabilityFlags edit_flags,
      qt::TouchEditingMenuClient* client);
  ~TouchEditingMenu() override;

 private Q_SLOTS:
  void OnActionTriggered(const QString& action);
  void OnVisibleChanged();
  void OnResize();

 private:
  TouchEditingMenu(QQuickItem* parent,
                   qt::TouchEditingMenuClient* client);
  bool Init(qt::EditCapabilityFlags edit_flags);

  // qt::TouchEditingMenu implementation
  void Show() override;
  void Hide() override;
  QSize GetSizeIncludingMargin() const override;
  void SetOrigin(const QPointF& origin) override;

  QPointer<QQuickItem> parent_;

  qt::TouchEditingMenuClient* client_;

  std::unique_ptr<QQuickItem> item_;

  bool inside_hide_ = false;
};

} // namespace uitk
} // namespace oxide

#endif // _OXIDE_QT_UITK_LIB_TOUCH_EDITING_MENU_H_
