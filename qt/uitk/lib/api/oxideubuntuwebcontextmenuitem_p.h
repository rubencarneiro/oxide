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

#ifndef _OXIDE_UITK_LIB_API_WEB_CONTEXT_MENU_ITEM_P_H_
#define _OXIDE_UITK_LIB_API_WEB_CONTEXT_MENU_ITEM_P_H_

#include <QList>
#include <QPointer>
#include <QtGlobal>

#include "qt/uitk/lib/api/oxideubuntuwebcontextmenuitem.h"

QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE

class OxideUbuntuWebContextMenu;

class OxideUbuntuWebContextMenuItemPrivate {
  Q_DISABLE_COPY(OxideUbuntuWebContextMenuItemPrivate)
  Q_DECLARE_PUBLIC(OxideUbuntuWebContextMenuItem)

 public:
  ~OxideUbuntuWebContextMenuItemPrivate();

  static OxideUbuntuWebContextMenuItemPrivate* get(
      OxideUbuntuWebContextMenuItem* d);

  static OxideUbuntuWebContextMenuItem* CreateStockItem(
      OxideUbuntuWebContextMenuItem::Action stock_action,
      OxideUbuntuWebContextMenuItem::Section section,
      QObject* action);

  static OxideUbuntuWebContextMenuItem* CreateItem(QObject* action);

  OxideUbuntuWebContextMenu* menu() const { return menu_.data(); }
  void set_menu(OxideUbuntuWebContextMenu* menu) { menu_ = menu; }

  void SetSection(OxideUbuntuWebContextMenuItem::Section section);

 private:
  OxideUbuntuWebContextMenuItemPrivate();

  OxideUbuntuWebContextMenuItem* q_ptr = nullptr;

  QPointer<OxideUbuntuWebContextMenu> menu_;

  QPointer<QObject> action_;

  OxideUbuntuWebContextMenuItem::Action stock_action_ =
      OxideUbuntuWebContextMenuItem::NoAction;

  OxideUbuntuWebContextMenuItem::Section section_ =
      OxideUbuntuWebContextMenuItem::NoSection;
};

#endif // _OXIDE_UITK_LIB_API_WEB_CONTEXT_MENU_ITEM_P_H_
