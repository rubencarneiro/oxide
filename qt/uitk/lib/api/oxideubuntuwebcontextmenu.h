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

#ifndef OXIDE_UBUNTU_WEB_CONTEXT_MENU
#define OXIDE_UBUNTU_WEB_CONTEXT_MENU

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtQml/QtQml>
#include <QtQml/QQmlListProperty>

#include <OxideUbuntuUITK/oxideubuntuglobal.h>
#include <OxideUbuntuUITK/oxideubuntuwebcontextmenuitem.h>

class OxideUbuntuWebContextMenuItem;
class OxideUbuntuWebContextMenuPrivate;

class OXIDE_UITK_EXPORT OxideUbuntuWebContextMenu : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged)
  Q_PROPERTY(QQmlListProperty<OxideUbuntuWebContextMenuItem> items READ items NOTIFY itemsChanged)

  Q_DISABLE_COPY(OxideUbuntuWebContextMenu)
  Q_DECLARE_PRIVATE(OxideUbuntuWebContextMenu)

 public:
  OxideUbuntuWebContextMenu(QObject* object = nullptr);
  ~OxideUbuntuWebContextMenu() Q_DECL_OVERRIDE;

  bool isEmpty() const;
  QQmlListProperty<OxideUbuntuWebContextMenuItem> items();

 public Q_SLOTS:
  int indexOfItem(OxideUbuntuWebContextMenuItem* item) const;
  int indexOfStockAction(OxideUbuntuWebContextMenuItem::Action action) const;

  void appendItem(OxideUbuntuWebContextMenuItem* item);
  int appendItemToSection(OxideUbuntuWebContextMenuItem::Section section,
                          OxideUbuntuWebContextMenuItem* item);
  void appendAction(QObject* action);
  int appendActionToSection(OxideUbuntuWebContextMenuItem::Section section,
                            QObject* action);

  void prependItem(OxideUbuntuWebContextMenuItem* item);
  int prependItemToSection(OxideUbuntuWebContextMenuItem::Section section,
                           OxideUbuntuWebContextMenuItem* item);
  void prependAction(QObject* action);
  int prependActionToSection(OxideUbuntuWebContextMenuItem::Section section,
                             QObject* action);

  void insertItemAt(int index, OxideUbuntuWebContextMenuItem* item);
  void insertActionAt(int index, QObject* action);
  int insertItemBefore(OxideUbuntuWebContextMenuItem::Action before,
                       OxideUbuntuWebContextMenuItem* item);
  int insertActionBefore(OxideUbuntuWebContextMenuItem::Action before,
                         QObject* action);
  int insertItemAfter(OxideUbuntuWebContextMenuItem::Action after,
                      OxideUbuntuWebContextMenuItem* item);
  int insertActionAfter(OxideUbuntuWebContextMenuItem::Action after,
                        QObject* action);

  void removeItem(OxideUbuntuWebContextMenuItem* item);
  void removeItemAt(int index);
  int removeAll();

 Q_SIGNALS:
  void isEmptyChanged();
  void itemsChanged();

 private:
  QScopedPointer<OxideUbuntuWebContextMenuPrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideUbuntuWebContextMenu)

#endif // OXIDE_UBUNTU_WEB_CONTEXT_MENU
