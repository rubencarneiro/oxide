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
#include <QString>
#include <QtAlgorithms>
#include <QUrl>

#include "oxideqquickwebview_p.h"
#include "oxideqquickwebview_p_p.h"

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
  d->webview_ = webview;
  d->entry_count_ = 0;
  d->current_index_ = -1;
  connect(webview, SIGNAL(navigationHistoryChanged()),
          SLOT(onNavigationHistoryChanged()));
}

OxideQQuickNavigationHistory::~OxideQQuickNavigationHistory() {
  Q_D(OxideQQuickNavigationHistory);
  qDeleteAll(d->entry_cache_);
  d->entry_cache_.clear();
}

void OxideQQuickNavigationHistory::onNavigationHistoryChanged() {
  Q_D(OxideQQuickNavigationHistory);

  OxideQQuickWebViewPrivate* adapter = OxideQQuickWebViewPrivate::get(d->webview_);
  int newCount = adapter->getNavigationEntryCount();
  if (newCount != d->entry_count_) {
    beginResetModel();
    d->entry_count_ = newCount;
    for (int i = 0; i < newCount; ++i) {
      int id = adapter->getNavigationEntryUniqueID(i);
      NavigationEntry* entry;
      if (d->entry_cache_.contains(id)) {
        entry = d->entry_cache_.value(id);
      } else {
        entry = new NavigationEntry;
        d->entry_cache_.insert(id, entry);
      }
      entry->url = adapter->getNavigationEntryUrl(i);
      entry->title = adapter->getNavigationEntryTitle(i);
      entry->timestamp = adapter->getNavigationEntryTimestamp(i);
    }
    endResetModel();
  } else if (d->current_index_ != -1) {
    int id = adapter->getNavigationEntryUniqueID(d->current_index_);
    NavigationEntry* entry;
    if (d->entry_cache_.contains(id)) {
      entry = d->entry_cache_.value(id);
    } else {
      entry = new NavigationEntry;
      d->entry_cache_.insert(id, entry);
    }
    QVector<int> roles;
    QUrl url = adapter->getNavigationEntryUrl(d->current_index_);
    if (url != entry->url) {
      entry->url = url;
      roles.append(OxideQQuickNavigationHistoryPrivate::Url);
    }
    QString title = adapter->getNavigationEntryTitle(d->current_index_);
    if (title != entry->title) {
      entry->title = title;
      roles.append(OxideQQuickNavigationHistoryPrivate::Title);
    }
    QDateTime timestamp = adapter->getNavigationEntryTimestamp(d->current_index_);
    if (timestamp != entry->timestamp) {
      entry->timestamp = timestamp;
      roles.append(OxideQQuickNavigationHistoryPrivate::Timestamp);
    }
    if (!roles.isEmpty()) {
      QModelIndex index = this->index(d->current_index_, 0);
      Q_EMIT dataChanged(index, index, roles);
    }
  }

  int newCurrentIndex = adapter->getNavigationCurrentEntryIndex();
  if (newCurrentIndex != d->current_index_) {
    d->current_index_ = newCurrentIndex;
    Q_EMIT currentIndexChanged();
  }
}

int OxideQQuickNavigationHistory::currentIndex() const {
  Q_D(const OxideQQuickNavigationHistory);
  return d->current_index_;
}

void OxideQQuickNavigationHistory::setCurrentIndex(int index) {
  Q_D(OxideQQuickNavigationHistory);
  if ((index < 0) || (index >= d->entry_count_)) {
    qWarning() << "Invalid index:" << index;
    return;
  }
  if (index != d->current_index_) {
    d->current_index_ = index;
    OxideQQuickWebViewPrivate* adapter = OxideQQuickWebViewPrivate::get(d->webview_);
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
  return d->entry_count_;
}

QVariant OxideQQuickNavigationHistory::data(const QModelIndex& index, int role) const {
  Q_D(const OxideQQuickNavigationHistory);
  if (!index.isValid()) {
    return QVariant();
  }
  int row = index.row();
  if ((row < 0) || (row >= d->entry_count_)) {
    return QVariant();
  }
  OxideQQuickWebViewPrivate* adapter = OxideQQuickWebViewPrivate::get(d->webview_);
  int id = adapter->getNavigationEntryUniqueID(row);
  NavigationEntry* entry;
  if (d->entry_cache_.contains(id)) {
    entry = d->entry_cache_.value(id);
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
