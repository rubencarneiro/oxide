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

#ifndef OXIDE_UBUNTU_WEB_CONTEXT_MENU_ITEM
#define OXIDE_UBUNTU_WEB_CONTEXT_MENU_ITEM

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtQml/QtQml>

#include <OxideUbuntuUITK/oxideubuntuglobal.h>

class OxideUbuntuWebContextMenuItemPrivate;

class OXIDE_UITK_EXPORT OxideUbuntuWebContextMenuItem : public QObject {
  Q_OBJECT
  Q_PROPERTY(Action stockAction READ stockAction CONSTANT)
  Q_PROPERTY(OxideUbuntuWebContextMenuItem::Section section READ section NOTIFY sectionChanged)
  Q_PROPERTY(QObject* action READ action WRITE setAction NOTIFY actionChanged)

  Q_ENUMS(Action)
  Q_ENUMS(Section)

  Q_DECLARE_PRIVATE(OxideUbuntuWebContextMenuItem)
  Q_DISABLE_COPY(OxideUbuntuWebContextMenuItem)

 public:
  enum Action {
    NoAction,

    ActionOpenLinkInNewTab,
    ActionOpenLinkInNewBackgroundTab,
    ActionOpenLinkInNewWindow,

    ActionCopyLinkLocation,
    ActionSaveLink,

    ActionOpenMediaInNewTab,
    ActionCopyMediaLocation,
    ActionSaveMedia,

    ActionCopyImage,

    ActionUndo,
    ActionRedo,

    ActionCut,
    ActionCopy,
    ActionPaste,
    ActionErase,
    ActionSelectAll
  };

  enum Section {
    NoSection,

    SectionOpenLink,
    SectionLink,
    SectionMedia,
    SectionUndo,
    SectionEditing,
    SectionCopy
  };

  OxideUbuntuWebContextMenuItem(QObject* parent = nullptr);
  ~OxideUbuntuWebContextMenuItem() Q_DECL_OVERRIDE;

  Action stockAction() const;
  Section section() const;

  QObject* action() const;
  void setAction(QObject* action);

 Q_SIGNALS:
  void actionChanged();
  void sectionChanged();

 private:
  OxideUbuntuWebContextMenuItem(OxideUbuntuWebContextMenuItemPrivate& dd,
                                QObject* parent = nullptr);

  QScopedPointer<OxideUbuntuWebContextMenuItemPrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideUbuntuWebContextMenuItem)
Q_DECLARE_METATYPE(OxideUbuntuWebContextMenuItem::Action)
Q_DECLARE_METATYPE(OxideUbuntuWebContextMenuItem::Section)

#endif // OXIDE_UBUNTU_WEB_CONTEXT_MENU_ITEM
