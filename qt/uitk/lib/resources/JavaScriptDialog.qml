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
  property string type
  property string defaultPromptValue
  property alias currentPromptValue : input.text
  property Item opener

  objectName: opener.objectName ? opener.objectName + "_JavaScriptDialog" : ""

  title: {
    if (type == "alert") {
      return i18n.tr("JavaScript Alert");
    } else if (type == "confirm") {
      return i18n.tr("JavaScript Confirmation");
    } else if (type == "prompt") {
      return i18n.tr("JavaScript Prompt");
    } else {
      console.error("Invalid JavaScriptDialog::type: " + type);
    }
  }

  signal response(bool success, string value)

  TextField {
    id: input
    objectName: root.objectName ? root.objectName + "_input" : ""
    visible: type == "prompt"
    text: defaultPromptValue
    onAccepted: response(true, text)
  }

  Button {
    objectName: root.objectName ? root.objectName + "_okButton" : ""
    text: i18n.tr("OK")
    color: theme.palette.normal.positive
    onClicked: response(true, input.text)
  }

  Button {
    objectName: root.objectName ? root.objectName + "_cancelButton" : ""
    visible: type == "confirm" || type == "prompt"
    text: i18n.tr("Cancel")
    onClicked: response(false, "")
  }

  Component.onCompleted: {
    if (type == "prompt") input.forceActiveFocus()
  }
}
