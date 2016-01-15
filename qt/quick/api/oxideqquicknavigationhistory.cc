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

#include "oxideqquicknavigationhistory_p.h"
#include "oxideqquicknavigationhistory_p_p.h"

#include <QDateTime>
#include <QDebug>
#include <QSet>
#include <QString>
#include <QtAlgorithms>
#include <QUrl>

#include "oxideqquickwebview.h"
#include "oxideqquickwebview_p.h"

struct NavigationEntry {
  QUrl url;
  QString title;
  QDateTime timestamp;
};

OxideQQuickNavigationHistory::OxideQQuickNavigationHistory(
    OxideQQuickWebView* webview) :
    QAbstractListModel(webview),
    d_ptr(new OxideQQuickNavigationHistoryPrivate) {
  Q_D(OxideQQuickNavigationHistory);
  d->q_ptr = this;
  d->webview = webview;
  d->entry_count = 0;
  d->current_index = -1;
}

OxideQQuickNavigationHistory::~OxideQQuickNavigationHistory() {
  Q_D(OxideQQuickNavigationHistory);
  qDeleteAll(d->entry_cache);
  d->entry_cache.clear();
}

void OxideQQuickNavigationHistory::onNavigationEntryCommitted() {
  Q_D(OxideQQuickNavigationHistory);

  OxideQQuickWebViewPrivate* adapter = OxideQQuickWebViewPrivate::get(d->webview);
  int newCount = adapter->getNavigationEntryCount();
  int index = adapter->getNavigationCurrentEntryIndex();
  if (newCount > d->entry_count) {
    beginInsertRows(QModelIndex(), index, index);
    d->entry_count = newCount;
    int id = adapter->getNavigationEntryUniqueID(index);
    if (!d->entry_cache.contains(id)) {
      NavigationEntry* entry = new NavigationEntry;
      d->entry_cache.insert(id, entry);
      entry->url = adapter->getNavigationEntryUrl(index);
      entry->title = adapter->getNavigationEntryTitle(index);
      entry->timestamp = adapter->getNavigationEntryTimestamp(index);
    }
    endInsertRows();
  }

  if (index != d->current_index) {
    d->current_index = index;
    Q_EMIT currentIndexChanged();
  }
}

void OxideQQuickNavigationHistory::onNavigationListPruned(bool from_front, int count) {
  Q_D(OxideQQuickNavigationHistory);

  int first;
  if (from_front) {
    first = 0;
  } else {
    first = d->entry_count - count;
  }
  int last = first + count - 1;
  beginRemoveRows(QModelIndex(), first, last);
  d->entry_count -= count;
  endRemoveRows();

  OxideQQuickWebViewPrivate* adapter = OxideQQuickWebViewPrivate::get(d->webview);
  QSet<int> ids;
  for (int i = 0; i < d->entry_count; ++i) {
    ids.insert(adapter->getNavigationEntryUniqueID(i));
  }
  Q_FOREACH(int id, d->entry_cache.keys()) {
    if (!ids.contains(id)) {
      delete d->entry_cache.take(id);
    }
  }
}

void OxideQQuickNavigationHistory::onNavigationEntryChanged(int index) {
  Q_D(OxideQQuickNavigationHistory);

  OxideQQuickWebViewPrivate* adapter = OxideQQuickWebViewPrivate::get(d->webview);
  int id = adapter->getNavigationEntryUniqueID(index);
  NavigationEntry* entry;
  if (d->entry_cache.contains(id)) {
    entry = d->entry_cache.value(id);
  } else {
    entry = new NavigationEntry;
    d->entry_cache.insert(id, entry);
  }
  QVector<int> roles;
  QUrl url = adapter->getNavigationEntryUrl(index);
  if (url != entry->url) {
    entry->url = url;
    roles.append(OxideQQuickNavigationHistoryPrivate::Url);
  }
  QString title = adapter->getNavigationEntryTitle(index);
  if (title != entry->title) {
    entry->title = title;
    roles.append(OxideQQuickNavigationHistoryPrivate::Title);
  }
  QDateTime timestamp = adapter->getNavigationEntryTimestamp(index);
  if (timestamp != entry->timestamp) {
    entry->timestamp = timestamp;
    roles.append(OxideQQuickNavigationHistoryPrivate::Timestamp);
  }
  if (!roles.isEmpty()) {
    QModelIndex modelIndex = this->index(index, 0);
    Q_EMIT dataChanged(modelIndex, modelIndex, roles);
  }
}

int OxideQQuickNavigationHistory::currentIndex() const {
  Q_D(const OxideQQuickNavigationHistory);
  return d->current_index;
}

void OxideQQuickNavigationHistory::setCurrentIndex(int index) {
  Q_D(OxideQQuickNavigationHistory);
  if ((index < 0) || (index >= d->entry_count)) {
    qWarning() << "Invalid index:" << index;
    return;
  }
  if (index != d->current_index) {
    d->current_index = index;
    OxideQQuickWebViewPrivate* adapter = OxideQQuickWebViewPrivate::get(d->webview);
    adapter->setNavigationCurrentEntryIndex(index);
    Q_EMIT currentIndexChanged();
  }
}

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
  return d->entry_count;
}

QVariant OxideQQuickNavigationHistory::data(const QModelIndex& index, int role) const {
  Q_D(const OxideQQuickNavigationHistory);
  if (!index.isValid()) {
    return QVariant();
  }
  int row = index.row();
  if ((row < 0) || (row >= d->entry_count)) {
    return QVariant();
  }
  OxideQQuickWebViewPrivate* adapter = OxideQQuickWebViewPrivate::get(d->webview);
  int id = adapter->getNavigationEntryUniqueID(row);
  NavigationEntry* entry;
  if (d->entry_cache.contains(id)) {
    entry = d->entry_cache.value(id);
  } else {
    return QVariant();
  }
  switch (role) {
  case OxideQQuickNavigationHistoryPrivate::Url:
    return entry->url;
  case OxideQQuickNavigationHistoryPrivate::Title:
    return entry->title;
  case OxideQQuickNavigationHistoryPrivate::Timestamp:
    return entry->timestamp;
  default:
    return QVariant();
  }
}
