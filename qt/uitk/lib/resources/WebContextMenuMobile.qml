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

Popups.Dialog {
  property var actions
  property bool isImage
  property var position
  property Item sourceItem
  property string title

  signal cancelled()

  Row {
    id: header
    spacing: units.gu(2)
    anchors {
      left: parent.left
      leftMargin: units.gu(2)
      right: parent.right
      rightMargin: units.gu(2)
    }
    height: units.gu(2 * titleLabel.lineCount + 3)
    visible: title

    Icon {
      width: units.gu(2)
      height: units.gu(2)
      anchors {
        top: parent.top
        topMargin: units.gu(2)
      }
      name: isImage ? "stock_image" : ""
      // work around the lack of a standard stock_link symbolic icon in the theme
      Component.onCompleted: {
        if (!name) {
          source = "stock_link.svg"
        }
      }
    }

    Label {
      id: titleLabel
      text: title
      width: parent.width - units.gu(4)
      anchors {
        top: parent.top
        topMargin: units.gu(2)
        bottom: parent.bottom
      }
      fontSize: "x-small"
      maximumLineCount: 2
      wrapMode: Text.Wrap
      height: contentHeight
    }
  }

  ListItems.ThinDivider {
    anchors {
      left: parent.left
      leftMargin: units.gu(2)
      right: parent.right
      rightMargin: units.gu(2)
    }
    visible: header.visible
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
        fontSize: "x-small"
        text: action.text
      }

      ListItems.ThinDivider {
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

  ListItems.Empty {
    height: units.gu(5)
    showDivider: false
    Label {
      anchors {
        left: parent.left
        leftMargin: units.gu(2)
        right: parent.right
        rightMargin: units.gu(2)
        verticalCenter: parent.verticalCenter
      }
      fontSize: "x-small"
      text: i18n.dtr("oxide-qt", "Cancel")
    }
    onTriggered: cancelled()
  }

  // adjust default dialog visuals to custom requirements for the context menu
  Binding {
    target: __foreground
    property: "margins"
    value: 0
  }
  Binding {
    target: __foreground
    property: "itemSpacing"
    value: 0
  }
}
