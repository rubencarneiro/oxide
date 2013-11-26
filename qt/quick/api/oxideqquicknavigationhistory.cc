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
#include "oxideqquickwebview_p.h"

#include <QDateTime>
#include <QString>
#include <QtAlgorithms>
#include <QUrl>

#include "qt/core/glue/oxide_qt_web_view_adapter.h"

struct NavigationEntry {
  QUrl url;
  QUrl virtualUrl;
  QString title;
  QString titleForDisplay;
  QDateTime timestamp;
};

OxideQQuickNavigationHistory::OxideQQuickNavigationHistory(
    oxide::qt::WebViewAdapter* adapter, OxideQQuickWebView* webview) :
    QAbstractListModel(webview),
    d_ptr(new OxideQQuickNavigationHistoryPrivate) {
  Q_D(OxideQQuickNavigationHistory);
  d->q_ptr = this;
  d->webview_adapter_ = adapter;
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
  Q_ASSERT(d->webview_adapter_ != NULL);

  int newCount = d->webview_adapter_->getNavigationEntryCount();
  if (newCount != d->entry_count_) {
    beginResetModel();
    d->entry_count_ = newCount;
    for (int i = 0; i < newCount; ++i) {
      int id = d->webview_adapter_->getNavigationEntryUniqueID(i);
      NavigationEntry* entry;
      if (d->entry_cache_.contains(id)) {
        entry = d->entry_cache_.value(id);
      } else {
        entry = new NavigationEntry;
        d->entry_cache_.insert(id, entry);
      }
      entry->url = d->webview_adapter_->getNavigationEntryUrl(i);
      entry->virtualUrl = d->webview_adapter_->getNavigationEntryVirtualUrl(i);
      entry->title = d->webview_adapter_->getNavigationEntryTitle(i);
      entry->titleForDisplay = d->webview_adapter_->getNavigationEntryTitleForDisplay(i);
      entry->timestamp = d->webview_adapter_->getNavigationEntryTimestamp(i);
    }
    endResetModel();
  } else if (d->current_index_ != -1) {
    int id = d->webview_adapter_->getNavigationEntryUniqueID(d->current_index_);
    NavigationEntry* entry = d->entry_cache_.value(id, NULL);
    if (entry != NULL) {
      QVector<int> roles;
      QUrl url = d->webview_adapter_->getNavigationEntryUrl(d->current_index_);
      if (url != entry->url) {
        entry->url = url;
        roles.append(OxideQQuickNavigationHistoryPrivate::Url);
      }
      QUrl virtualUrl = d->webview_adapter_->getNavigationEntryVirtualUrl(d->current_index_);
      if (virtualUrl != entry->virtualUrl) {
        entry->virtualUrl = virtualUrl;
        roles.append(OxideQQuickNavigationHistoryPrivate::VirtualUrl);
      }
      QString title = d->webview_adapter_->getNavigationEntryTitle(d->current_index_);
      if (title != entry->title) {
        entry->title = title;
        roles.append(OxideQQuickNavigationHistoryPrivate::Title);
      }
      QString titleForDisplay = d->webview_adapter_->getNavigationEntryTitleForDisplay(d->current_index_);
      if (titleForDisplay != entry->titleForDisplay) {
        entry->titleForDisplay = titleForDisplay;
        roles.append(OxideQQuickNavigationHistoryPrivate::TitleForDisplay);
      }
      QDateTime timestamp = d->webview_adapter_->getNavigationEntryTimestamp(d->current_index_);
      if (timestamp != entry->timestamp) {
        entry->timestamp = timestamp;
        roles.append(OxideQQuickNavigationHistoryPrivate::Timestamp);
      }
      if (!roles.isEmpty()) {
        QModelIndex index = this->index(d->current_index_, 0);
        Q_EMIT dataChanged(index, index, roles);
      }
    }
  }

  int newCurrentIndex = d->webview_adapter_->getNavigationCurrentEntryIndex();
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
    return;
  }
  if (index != d->current_index_) {
    d->current_index_ = index;
    d->webview_adapter_->setNavigationCurrentEntryIndex(index);
    Q_EMIT currentIndexChanged();
  }
}

QHash<int, QByteArray> OxideQQuickNavigationHistory::roleNames() const {
  static QHash<int, QByteArray> roles;
  if (roles.isEmpty()) {
    roles[OxideQQuickNavigationHistoryPrivate::Url] = "url";
    roles[OxideQQuickNavigationHistoryPrivate::VirtualUrl] = "virtualUrl";
    roles[OxideQQuickNavigationHistoryPrivate::Title] = "title";
    roles[OxideQQuickNavigationHistoryPrivate::TitleForDisplay] = "titleForDisplay";
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
  int id = d->webview_adapter_->getNavigationEntryUniqueID(index.row());
  NavigationEntry* entry;
  if (d->entry_cache_.contains(id)) {
    entry = d->entry_cache_.value(id);
  } else {
    return QVariant();
  }
  switch (role) {
  case OxideQQuickNavigationHistoryPrivate::Url:
    return entry->url;
  case OxideQQuickNavigationHistoryPrivate::VirtualUrl:
    return entry->virtualUrl;
  case OxideQQuickNavigationHistoryPrivate::Title:
    return entry->title;
  case OxideQQuickNavigationHistoryPrivate::TitleForDisplay:
    return entry->titleForDisplay;
  case OxideQQuickNavigationHistoryPrivate::Timestamp:
    return entry->timestamp;
  default:
    return QVariant();
  }
}
