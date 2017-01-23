// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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
  property var model

  id: root
  objectName: parent.objectName ? parent.objectName + "_WebPopupMenu" : ""

  signal selectedItem(int index)

  caller: parent

  contentWidth: Math.min(parent.width - units.gu(10), units.gu(40))

  property real listContentHeight: 0 // intermediate property to avoid binding loop
  contentHeight: Math.min(parent.height - units.gu(10), listContentHeight)

  Rectangle {
    anchors.fill: parent
    color: "#ececec"
  }

  ListView {
    clip: true
    width: root.contentWidth
    height: root.contentHeight

    objectName: root.objectName ? root.objectName + "_ListView" : ""

    model: root.model

    delegate: ListItem {
      objectName: root.objectName ? root.objectName + "_item" + index : ""

      ListItemLayout {
        title.text: model.text
      }
      enabled: model.enabled
      selected: model.checked
      onClicked: selectedItem(model.index)
    }

    section.property: "group"
    section.delegate: ListItems.Header {
      text: section
    }

    onContentHeightChanged: root.listContentHeight = contentHeight
  }
}
