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
import Ubuntu.Components.ListItems 1.3 as ListItems
import Ubuntu.Components.Popups 1.3 as Popups

Popups.Popover {
  property var actions
  property bool isImage
  property var position
  property Item sourceItem
  property string title

  signal cancelled()

  QtObject {
    id: internal

    readonly property int lastEnabledActionIndex: {
      var last = -1
      for (var i in actions) {
        if (actions[i].enabled) {
          last = i
        }
      }
      return last
    }
  }

  Rectangle {
    anchors.fill: parent
    color: "#ececec"
  }

  Column {
    anchors {
      left: parent.left
      right: parent.right
    }

    Label {
      id: titleLabel

      anchors {
        left: parent.left
        leftMargin: units.gu(2)
        right: parent.right
        rightMargin: units.gu(2)
      }

      text: title
      height: units.gu(5)
      visible: text
      fontSize: "x-small"
      color: "#888888"
      elide: Text.ElideRight
      verticalAlignment: Text.AlignVCenter
    }

    ListItems.ThinDivider {
      anchors {
        left: parent.left
        leftMargin: units.gu(2)
        right: parent.right
        rightMargin: units.gu(2)
      }
      visible: titleLabel.visible
    }

    Repeater {
      model: actions
      delegate: ListItems.Empty {
        action: modelData
        visible: action.enabled
        showDivider: false

        height: units.gu(5)

        Label {
          anchors {
            left: parent.left
            leftMargin: units.gu(2)
            right: parent.right
            rightMargin: units.gu(2)
            verticalCenter: parent.verticalCenter
          }
          fontSize: "small"
          text: action.text
        }

        ListItems.ThinDivider {
          visible: index < internal.lastEnabledActionIndex
          anchors {
            left: parent.left
            leftMargin: units.gu(2)
            right: parent.right
            rightMargin: units.gu(2)
            bottom: parent.bottom
          }
        }
      }
    }
  }

  Item {
    id: positioner
    visible: false
    parent: sourceItem
    x: position.x
    y: position.y
  }

  caller: positioner

  // Override default implementation to prevent context menu from stealing
  // active focus when shown (https://launchpad.net/bugs/1526884).
  function show() {
    visible = true
    __foreground.show()
  }

  Binding {
    // Ensure the context menu doesn’t steal focus from
    // the webview when one of its actions is activated
    // (https://launchpad.net/bugs/1526884).
    target: __foreground
    property: "activeFocusOnPress"
    value: false
  }
}
