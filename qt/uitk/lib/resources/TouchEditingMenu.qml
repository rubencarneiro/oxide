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

UbuntuShape {
  id: root

  property int editFlags: 0

  signal actionTriggered(string action)

  // Keep these in sync with qt/core/glue/edit_capability_flags.h
  // Note, this implementation is temporary - when we implement
  // https://launchpad.net/bugs/1668412, this component will be supplied
  // with a list of Action instances in the same way that the context menu is
  readonly property int kEditCapabilityCut: 4
  readonly property int kEditCapabilityCopy: 8
  readonly property int kEditCapabilityPaste: 16

  visible: false
  aspect: UbuntuShape.DropShadow
  backgroundColor: "white"

  readonly property int padding: units.gu(1)
  width: actionsRow.width + padding * 2
  height: childrenRect.height + padding * 2

  ActionList {
    id: actions
    Action {
      name: "selectall"
      text: i18n.dtr('ubuntu-ui-toolkit', "Select All")
      iconName: "edit-select-all"
      enabled: true
      visible: true
      onTriggered: actionTriggered(name)
    }
    Action {
      name: "cut"
      text: i18n.dtr('ubuntu-ui-toolkit', "Cut")
      iconName: "edit-cut"
      enabled: editFlags & kEditCapabilityCut
      visible: enabled
      onTriggered: actionTriggered(name)
    }
    Action {
      name: "copy"
      text: i18n.dtr('ubuntu-ui-toolkit', "Copy")
      iconName: "edit-copy"
      enabled: editFlags & kEditCapabilityCopy
      visible: enabled
      onTriggered: actionTriggered(name)
    }
    Action {
      name: "paste"
      text: i18n.dtr('ubuntu-ui-toolkit', "Paste")
      iconName: "edit-paste"
      enabled: editFlags & kEditCapabilityPaste
      visible: enabled
      onTriggered: actionTriggered(name)
    }
  }

  Row {
    id: actionsRow
    x: parent.padding
    y: parent.padding
    width: {
      // work around what seems to be a bug in Row’s childrenRect.width
      var w = 0
      for (var i in visibleChildren) {
        w += visibleChildren[i].width
      }
      return w
    }
    height: units.gu(6)

    Repeater {
      model: actions.children
      AbstractButton {
        anchors {
          top: parent.top
          bottom: parent.bottom
        }
        width: Math.max(units.gu(5), implicitWidth) + units.gu(2)
        action: modelData
        styleName: "ToolbarButtonStyle"
        activeFocusOnPress: false
      }
    }
  }
}
