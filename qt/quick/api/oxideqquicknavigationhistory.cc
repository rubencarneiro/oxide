// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#include "oxideqquicknavigationhistory.h"
#include "oxideqquicknavigationhistory_p.h"

#include <QDateTime>
#include <QDebug>
#include <QMetaType>
#include <QString>
#include <QUrl>

#include "oxideqquicknavigationitem.h"
#include "oxideqquicknavigationitem_p.h"

using oxide::qt::NavigationHistory;
using oxide::qt::NavigationHistoryItem;
using oxide::qt::WebContentsID;

struct OxideQQuickNavigationHistoryPrivate::ModelHistoryItem {
  QUrl url;
  QString title;
  QDateTime timestamp;
};

OxideQQuickNavigationHistoryPrivate::OxideQQuickNavigationHistoryPrivate(
    OxideQQuickNavigationHistory* q)
    : q_ptr(q),
      model_items_need_rebuilding_(true) {}

void OxideQQuickNavigationHistoryPrivate::ensureModelItemsAreBuilt() {
  if (!model_items_need_rebuilding_) {
    return;
  }

  Q_ASSERT(model_items_.isEmpty());
  model_items_need_rebuilding_ = false;

  for (int i = 0; i < navigation_history_->getItemCount(); ++i) {
    QExplicitlySharedDataPointer<NavigationHistoryItem> item =
        navigation_history_->getItemAtIndex(i);
    ModelHistoryItem model_item;
    model_item.url = item->url();
    model_item.title = item->title();
    model_item.timestamp = item->timestamp();
    model_items_.push_back(model_item);
  }
}

OxideQQuickNavigationItem
OxideQQuickNavigationHistoryPrivate::constructItemForIndex(int index) {
  Q_Q(OxideQQuickNavigationHistory);

  QExplicitlySharedDataPointer<NavigationHistoryItem> history_item =
      navigation_history_->getItemAtIndex(index);
  
  return OxideQQuickNavigationItem(
      *new OxideQQuickNavigationItemData(history_item.data()));
}

void OxideQQuickNavigationHistoryPrivate::NavigationHistoryChanged() {
  Q_Q(OxideQQuickNavigationHistory);

  model_items_need_rebuilding_ = false;
  q->beginResetModel();
  model_items_need_rebuilding_ = true;
  model_items_.clear();
  q->endResetModel();

  Q_EMIT q->changed();
  Q_EMIT q->currentIndexChanged();
}

OxideQQuickNavigationHistoryPrivate
    ::~OxideQQuickNavigationHistoryPrivate() = default;

// static
OxideQQuickNavigationHistoryPrivate* OxideQQuickNavigationHistoryPrivate::get(
    OxideQQuickNavigationHistory* q) {
  return q->d_func();
}

void OxideQQuickNavigationHistoryPrivate::init(WebContentsID web_contents_id) {
  navigation_history_->init(web_contents_id);
}

/*!
\class OxideQQuickNavigationHistory
\inmodule OxideQtQuick
\inheaderfile oxideqquicknavigationhistory.h

\brief Represents a view's navigation history
*/

/*!
\qmltype NavigationHistory
\inqmlmodule com.canonical.Oxide 1.0
\instantiates OxideQQuickNavigationHistory

\brief Represents a view's navigation history

NavigationHistory represents a view's navigation history. It provides lists of
NavigationItems via backItems, forwardItems and \l{items}, which provide details
about individual history entries. It's also possible to navigate to entries in
the navigation history by calling goBack, goForward or goToOffset, or by
manipulating the currentItem or currentItemIndex properties directly.

NavigationHistory is also a list model, although this functionality is
deprecated and should not be used in newly written code.
*/

/*!
\qmlsignal void NavigationHistory::changed()
\since OxideQt 1.19

This signal is emitted whenever the navigation history changes. This can happen
multiple times during a single navigation.
*/

QHash<int, QByteArray> OxideQQuickNavigationHistory::roleNames() const {
  static QHash<int, QByteArray> roles;
  if (roles.isEmpty()) {
    roles[OxideQQuickNavigationHistoryPrivate::Url] = "url";
    roles[OxideQQuickNavigationHistoryPrivate::Title] = "title";
    roles[OxideQQuickNavigationHistoryPrivate::Timestamp] = "timestamp";
  }
  return roles;
}

int OxideQQuickNavigationHistory::rowCount(const QModelIndex& parent) const {
  Q_UNUSED(parent);
  Q_D(const OxideQQuickNavigationHistory);

  const_cast<OxideQQuickNavigationHistory*>(this)
      ->d_func()->ensureModelItemsAreBuilt();

  return d->model_items_.size();
}

QVariant OxideQQuickNavigationHistory::data(const QModelIndex& index,
                                            int role) const {
  Q_D(const OxideQQuickNavigationHistory);

  const_cast<OxideQQuickNavigationHistory*>(this)
      ->d_func()->ensureModelItemsAreBuilt();

  if (!index.isValid()) {
    return QVariant();
  }

  int row = index.row();
  if ((row < 0) || (row >= d->model_items_.size())) {
    return QVariant();
  }

  const OxideQQuickNavigationHistoryPrivate::ModelHistoryItem& item =
      d->model_items_[row];

  switch (role) {
    case OxideQQuickNavigationHistoryPrivate::Url:
      return item.url;
    case OxideQQuickNavigationHistoryPrivate::Title:
      return item.title;
    case OxideQQuickNavigationHistoryPrivate::Timestamp:
      return item.timestamp;
    default:
      return QVariant();
  }
}

OxideQQuickNavigationHistory::OxideQQuickNavigationHistory()
    : d_ptr(new OxideQQuickNavigationHistoryPrivate(this)) {
  Q_D(OxideQQuickNavigationHistory);
  d->navigation_history_ = NavigationHistory::create(d, this);
}

OxideQQuickNavigationHistory::~OxideQQuickNavigationHistory() = default;

/*!
\qmlproperty list<NavigationItem> NavigationHistory::backItems
\since OxideQt 1.19

This is the list of navigation items preceding the currentItem. The order is
such that the next item to be navigated to by calling goBack is the first item
in the list.

Any items in the list referenced by the QML engine will be kept alive until they
go out of scope.

The notify signal for this property will be emitted whenever the list changes.
When this happens, items that are removed but still referenced by the QML engine
will become stale (they are still valid objects, but can't be used to initiate a
history navigation by setting currentItem). Properties of items that persist
between changes may also be updated.

\note The notify signal for this is \l{changed}
*/

QVariantList OxideQQuickNavigationHistory::backItems() {
  Q_D(OxideQQuickNavigationHistory);

  int current = d->navigation_history_->getCurrentItemIndex();
  if (current < 1) {
    return QVariantList();
  }

  QVariantList rv;
  for (int i = current - 1; i >= 0; --i) {
    rv.push_back(QVariant::fromValue(d->constructItemForIndex(i)));
  }

  return rv;
}

/*!
\qmlproperty list<NavigationItem> NavigationHistory::forwardItems
\since OxideQt 1.19

This is the list of navigation items following the currentItem.

Any items in the list referenced by the QML engine will be kept alive until they
go out of scope.

The notify signal for this property will be emitted whenever the list changes.
When this happens, items that are removed but still referenced by the QML engine
will become stale (they are still valid objects, but can't be used to initiate a
history navigation by setting currentItem). Properties of items that persist
between changes may also be updated.

\note The notify signal for this is \l{changed}
*/

QVariantList OxideQQuickNavigationHistory::forwardItems() {
  Q_D(OxideQQuickNavigationHistory);

  int current = d->navigation_history_->getCurrentItemIndex();
  int count = d->navigation_history_->getItemCount();
  if (current >= count - 1) {
    return QVariantList();
  }

  QVariantList rv;
  for (int i = current + 1; i < count; ++i) {
    rv.push_back(QVariant::fromValue(d->constructItemForIndex(i)));
  }

  return rv;
}

/*!
\qmlproperty list<NavigationItem> NavigationHistory::items
\since OxideQt 1.19

This is the complete list of navigation items, including the currentItem.

Any items in the list referenced by the QML engine will be kept alive until they
go out of scope.

The notify signal for this property will be emitted whenever the list changes.
When this happens, items that are removed but still referenced by the QML engine
will become stale (they are still valid objects, but can't be used to initiate a
history navigation by setting currentItem). Properties of items that persist
between changes may also be updated.

\note The notify signal for this is \l{changed}
*/

QVariantList OxideQQuickNavigationHistory::items() {
  Q_D(OxideQQuickNavigationHistory);

  QVariantList rv;
  for (int i = 0; i < d->navigation_history_->getItemCount(); ++i) {
    rv.push_back(QVariant::fromValue(d->constructItemForIndex(i)));
  }

  return rv;
}

/*!
\qmlproperty NavigationItem NavigationHistory::currentItem
\since OxideQt 1.19

This is the current navigation item. If there is a browser-initiated history
navigation in progress, this item will be the pending navigation item.
Otherwise it will be the current committed navigation item, or null if no
navigation has committed yet in this view.

The item will be kept alive as long as it is referenced by the QML engine.

Setting this property to another valid NavigationItem will initiate a history
navigation to that item.

\note The notify signal for this is \l{changed}.
*/

QVariant OxideQQuickNavigationHistory::currentItem() {
  Q_D(OxideQQuickNavigationHistory);

  int current = d->navigation_history_->getCurrentItemIndex();
  if (current < 0) {
    // We return a null QVariant with the type VoidStar, as this gets converted
    // to null in QML engine
    return QVariant(static_cast<QVariant::Type>(QMetaType::VoidStar));
  }

  return QVariant::fromValue(d->constructItemForIndex(current));
}

void OxideQQuickNavigationHistory::setCurrentItem(const QVariant& item) {
  Q_D(OxideQQuickNavigationHistory);

  if (item.isNull()) {
    qWarning() <<
        "OxideQQuickNavigationHistory::setCurrentItem: Cannot navigate to "
        "null item";
    return;
  }

  if (item.userType() != qMetaTypeId<OxideQQuickNavigationItem>()) {
    qWarning() << "OxideQQuickNavigationHistory::setCurrentItem: Invalid type";
    return;
  }

  OxideQQuickNavigationItem qitem = item.value<OxideQQuickNavigationItem>();
  OxideQQuickNavigationItemData* data =
      OxideQQuickNavigationItemData::get(qitem);

  int index = d->navigation_history_->getItemIndex(data->item());
  if (index < 0) {
    qWarning() <<
        "OxideQQuickNavigationHistory::setCurrentItem: The supplied navigation "
        "item doesn't exist in this navigation history";
    return;
  }

  d->navigation_history_->goToIndex(index);
}

/*!
\qmlproperty int NavigationHistory::currentItemIndex
\since OxideQt 1.19

This is the index of the current NavigationItem (currentItem) within the
complete list of items (\l{items}).

Setting this property to a new index will initiate a history navigation to the
NavigationItem at that index.

\note The notify signal for this is \l{changed}.
*/

/*!
\qmlproperty int NavigationHistory::currentIndex
\deprecated
*/

int OxideQQuickNavigationHistory::currentItemIndex() const {
  Q_D(const OxideQQuickNavigationHistory);
  return d->navigation_history_->getCurrentItemIndex();
}

void OxideQQuickNavigationHistory::setCurrentItemIndex(int index) {
  Q_D(OxideQQuickNavigationHistory);
  if ((index < 0) || (index >= d->navigation_history_->getItemCount())) {
    qWarning()
        << "OxideQQuickNavigationHistory::setCurrentItemIndex: Invalid index: "
        << index;
    return;
  }

  if (index == currentItemIndex()) {
    return;
  }

  d->navigation_history_->goToIndex(index);
}

/*!
\qmlproperty bool NavigationHistory::canGoBack
\since OxideQt 1.19

This property will be true if the application can navigate to the previous
NavigationItem by calling goBack, otherwise it will be false.

\note The notify signal for this is \l{changed}.
*/

bool OxideQQuickNavigationHistory::canGoBack() const {
  Q_D(const OxideQQuickNavigationHistory);
  return d->navigation_history_->canGoBack();
}

/*!
\qmlproperty bool NavigationHistory::canGoForward
\since OxideQt 1.19

This property will be true if the application can navigate to the next
NavigationItem by calling goForward, otherwise it will be false.

\note The notify signal for this is \l{changed}.
*/

bool OxideQQuickNavigationHistory::canGoForward() const {
  Q_D(const OxideQQuickNavigationHistory);
  return d->navigation_history_->canGoForward();
}

/*!
\qmlmethod void NavigationHistory::goBack()
\since OxideQt 1.19

Navigate to the previous NavigationItem. If there isn't one (canGoBack is false),
then calling this function will do nothing.
*/

void OxideQQuickNavigationHistory::goBack() {
  Q_D(OxideQQuickNavigationHistory);

  if (!canGoBack()) {
    qWarning() << "OxideQQuickNavigationHistory::goBack: Cannot go back";
    return;
  }

  d->navigation_history_->goBack();
}

/*!
\qmlmethod void NavigationHistory::goForward()
\since OxideQt 1.19

Navigate to the next NavigationItem. If there isn't one (canGoForward is false),
then calling this function will do nothing.
*/

void OxideQQuickNavigationHistory::goForward() {
  Q_D(OxideQQuickNavigationHistory);

  if (!canGoForward()) {
    qWarning() << "OxideQQuickNavigationHistory::goForward: Cannot go forward";
    return;
  }

  d->navigation_history_->goForward();
}

/*!
\qmlmethod void NavigationHistory::goToOffset(int offset)
\since OxideQt 1.19

Navigate to the NavigationItem at the specified \a{offset} from
\l{currentItemIndex}. Setting this to a negative number will navigate backwards
through history.

This will do nothing if there is no NavigationItem at the specified \a{offset}.
*/

void OxideQQuickNavigationHistory::goToOffset(int offset) {
  Q_D(OxideQQuickNavigationHistory);

  if (!d->navigation_history_->canGoToOffset(offset)) {
    qWarning() <<
        "OxideQQuickNavigationHistory::goToOffset: Cannot go to offset";
    return;
  }

  d->navigation_history_->goToOffset(offset);
}
