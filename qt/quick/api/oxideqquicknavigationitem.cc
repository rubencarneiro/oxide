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

#include "oxideqquicknavigationitem.h"
#include "oxideqquicknavigationitem_p.h"

#include "qt/core/glue/navigation_history.h"

using oxide::qt::NavigationHistoryItem;

OxideQQuickNavigationItemData::OxideQQuickNavigationItemData()
    : item_(new NavigationHistoryItem()) {}

OxideQQuickNavigationItemData::OxideQQuickNavigationItemData(
    NavigationHistoryItem* item)
    : item_(item) {}

OxideQQuickNavigationItemData::~OxideQQuickNavigationItemData() = default;

// static
OxideQQuickNavigationItem OxideQQuickNavigationItemData::createForTesting(
    const QUrl& url,
    const QUrl& original_url,
    const QString& title,
    const QDateTime& timestamp) {
  return OxideQQuickNavigationItem(
      *new OxideQQuickNavigationItemData(
          new NavigationHistoryItem(url,
                                    original_url,
                                    title,
                                    timestamp)));
}

// static
OxideQQuickNavigationItemData* OxideQQuickNavigationItemData::get(
    const OxideQQuickNavigationItem& q) {
  return q.d.data();
}

/*!
\class OxideQQuickNavigationItem
\inmodule OxideQtQuick
\inheaderfile oxideqquicknavigationitem.h
\since OxideQt 1.19

\brief Represents an entry in the navigation history
*/

/*!
\qmltype NavigationItem
\inqmlmodule com.canonical.Oxide 1.19
\instantiates OxideQQuickNavigationItem

\brief Represents an entry in the navigation history

NavigationItem represents an entry in a view's navigation history.
*/

OxideQQuickNavigationItem::OxideQQuickNavigationItem(
    OxideQQuickNavigationItemData& d)
    : d(&d) {}

OxideQQuickNavigationItem::OxideQQuickNavigationItem()
    : d(new OxideQQuickNavigationItemData()) {}

OxideQQuickNavigationItem::OxideQQuickNavigationItem(
    const OxideQQuickNavigationItem& other)
    : d(new OxideQQuickNavigationItemData(other.d->item_.data())) {}

OxideQQuickNavigationItem::~OxideQQuickNavigationItem() = default;

/*!
\qmlproperty url NavigationItem::url

The URL of the navigation item. This is the actual URL that was loaded the last
time that this navigation item was committed, and may be different to
originalUrl if there were redirects.
*/

QUrl OxideQQuickNavigationItem::url() const {
  return d->item_->url();
}

/*!
\qmlproperty url NavigationItem::originalUrl

The original request URL for the navigation item. The final URL that is loaded
may be different to the original request URL if the navigation is redirected.
*/

QUrl OxideQQuickNavigationItem::originalUrl() const {
  return d->item_->original_url();
}

/*!
\qmlproperty string NavigationItem::title

The title of the page associated with this navigation item. This can be an empty
string if the page has no title.
*/

QString OxideQQuickNavigationItem::title() const {
  return d->item_->title();
}

/*!
\qmlproperty Date NavigationItem::timstamp

The timestamp of the last time that this navigation item was committed.
*/

QDateTime OxideQQuickNavigationItem::timestamp() const {
  return d->item_->timestamp();
}

OxideQQuickNavigationItem& OxideQQuickNavigationItem::operator=(
    const OxideQQuickNavigationItem& other) {
  d->item_ = other.d->item_;
  return *this;
}

bool OxideQQuickNavigationItem::operator==(
    const OxideQQuickNavigationItem& other) const {
  return d->item_ == other.d->item_;
}

bool OxideQQuickNavigationItem::operator!=(
    const OxideQQuickNavigationItem& other) const {
  return !(*this == other);
}
