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

#ifndef _OXIDE_QT_QUICK_API_NAVIGATION_HISTORY_P_H_
#define _OXIDE_QT_QUICK_API_NAVIGATION_HISTORY_P_H_

#include <QAbstractListModel>
#include <QScopedPointer>
#include <QtQml>

namespace oxide {
namespace qt {
class WebViewAdapter;
}
}

class OxideQQuickNavigationHistoryPrivate;
class OxideQQuickWebView;

class OxideQQuickNavigationHistory : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

 public:
  OxideQQuickNavigationHistory(OxideQQuickWebView* webview);
  ~OxideQQuickNavigationHistory();

  // reimplemented from QAbstractListModel
  QHash<int, QByteArray> roleNames() const;
  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  QVariant data(const QModelIndex& index, int role) const;

  int currentIndex() const;
  void setCurrentIndex(int index);

Q_SIGNALS:
  void currentIndexChanged();

private Q_SLOTS:
  friend class OxideQQuickWebViewPrivate;
  void onNavigationHistoryChanged();
  void onNavigationEntryCommitted();
  void onNavigationListPruned(bool from_front, int count);
  void onNavigationEntryChanged(int index);

private:
  Q_DISABLE_COPY(OxideQQuickNavigationHistory)
  QScopedPointer<OxideQQuickNavigationHistoryPrivate> d_ptr;
  Q_DECLARE_PRIVATE(OxideQQuickNavigationHistory)
};

QML_DECLARE_TYPE(OxideQQuickNavigationHistory)

#endif // _OXIDE_QT_QUICK_API_NAVIGATION_HISTORY_P_H_
