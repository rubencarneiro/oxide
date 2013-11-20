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
#include "oxideqquickwebview_p.h"

#include "qt/core/glue/oxide_qt_web_view_adapter.h"

OxideQQuickNavigationHistory::OxideQQuickNavigationHistory(QObject* parent) :
    QAbstractListModel(parent),
    webview_(NULL),
    webview_adapter_(NULL) {
}

OxideQQuickNavigationHistory::~OxideQQuickNavigationHistory() {}

void OxideQQuickNavigationHistory::setWebView(OxideQQuickWebView* webview) {
  webview_ = webview;
  connect(webview_, SIGNAL(navigationHistoryChanged()),
          SLOT(onNavigationHistoryChanged()));
}

void OxideQQuickNavigationHistory::setWebViewApdater(
    oxide::qt::WebViewAdapter *adapter) {
  webview_adapter_ = adapter;
}

void OxideQQuickNavigationHistory::onNavigationHistoryChanged() {
  // FIXME: batch calls to this slot before resetting the model
  beginResetModel();
  endResetModel();
}

int OxideQQuickNavigationHistory::currentIndex() const {
  // TODO: implement me
  return 0;
}

QHash<int, QByteArray> OxideQQuickNavigationHistory::roleNames() const {
  static QHash<int, QByteArray> roles;
  if (roles.isEmpty()) {
    roles[Url] = "url";
    roles[VirtualUrl] = "virtualUrl";
    roles[Title] = "title";
    roles[TitleForDisplay] = "titleForDisplay";
    roles[Timestamp] = "timestamp";
  }
  return roles;
}

int OxideQQuickNavigationHistory::rowCount(const QModelIndex& parent) const {
  Q_UNUSED(parent);
  Q_ASSERT(webview_adapter_ != NULL);
  return webview_adapter_->getNavigationEntryCount();
}

QVariant OxideQQuickNavigationHistory::data(const QModelIndex& index, int role) const {
  // TODO: implement me
  return QVariant();
}
