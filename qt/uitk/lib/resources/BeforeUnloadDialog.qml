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

import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3 as Popups

Popups.Dialog {
  id: root
  property Item opener

  objectName: opener.objectName ? opener.objectName + "_BeforeUnloadDialog" : ""

  title: i18n.tr("Confirm Navigation")
  text: i18n.tr("Changes you made may not be saved.")

  signal response(bool proceed)

  Button {
    objectName: root.objectName ? root.objectName + "_leaveButton" : ""
    text: i18n.tr("Leave")
    color: theme.palette.normal.positive
    onClicked: response(true)
  }

  Button {
    objectName: root.objectName ? root.objectName + "_stayButton" : ""
    text: i18n.tr("Stay")
    onClicked: response(false)
  }
}
