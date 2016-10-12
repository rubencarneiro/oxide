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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA

#include "oxide_qml_navigation_item.h"

namespace oxide {
namespace qmlplugin {

NavigationItem::NavigationItem(QObject* parent)
    : QQmlValueTypeBase<OxideQQuickNavigationItem>(
          qMetaTypeId<OxideQQuickNavigationItem>(),
          parent) {}

NavigationItem::~NavigationItem() = default;

QUrl NavigationItem::url() const {
  return v.url();
}

QUrl NavigationItem::originalUrl() const {
  return v.url();
}

QString NavigationItem::title() const {
  return v.title();
}

QDateTime NavigationItem::timestamp() const {
  return v.timestamp();
}

QString NavigationItem::toString() const {
  return QString();
}

bool NavigationItem::isEqual(const QVariant& other) const {
  if (other.userType() != qMetaTypeId<OxideQQuickNavigationItem>()) {
    return false;
  }

  return v == other.value<OxideQQuickNavigationItem>();
}

} // namespace qmlplugin
} // namespace oxide
