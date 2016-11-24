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

#ifndef OXIDE_QTQUICK_NAVIGATION_HISTORY
#define OXIDE_QTQUICK_NAVIGATION_HISTORY

#include <QtCore/QAbstractListModel>
#include <QtCore/QScopedPointer>
#include <QtCore/QVariantList>
#include <QtCore/QVariant>
#include <QtQml/QtQml>

#include <OxideQtQuick/oxideqquickglobal.h>

class OxideQQuickNavigationHistoryPrivate;

class OXIDE_QTQUICK_EXPORT OxideQQuickNavigationHistory
    : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(QVariantList backItems READ backItems NOTIFY changed REVISION 1)
  Q_PROPERTY(QVariantList forwardItems READ forwardItems NOTIFY changed REVISION 1)
  Q_PROPERTY(QVariantList items READ items NOTIFY changed REVISION 1)

  Q_PROPERTY(QVariant currentItem READ currentItem WRITE setCurrentItem NOTIFY changed REVISION 1)
  Q_PROPERTY(int currentItemIndex READ currentItemIndex WRITE setCurrentItemIndex NOTIFY changed REVISION 1)
  Q_PROPERTY(int currentIndex READ currentItemIndex WRITE setCurrentItemIndex NOTIFY changed)

  Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY changed REVISION 1)
  Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY changed REVISION 1)

  Q_DECLARE_PRIVATE(OxideQQuickNavigationHistory)
  Q_DISABLE_COPY(OxideQQuickNavigationHistory)

 public:
  ~OxideQQuickNavigationHistory() Q_DECL_OVERRIDE;

  QVariantList backItems();
  QVariantList forwardItems();
  QVariantList items();

  QVariant currentItem();
  void setCurrentItem(const QVariant& item);

  int currentItemIndex() const;
  void setCurrentItemIndex(int index);

  bool canGoBack() const;
  bool canGoForward() const;

 public Q_SLOTS:
  Q_REVISION(1) void goBack();
  Q_REVISION(1) void goForward();

  Q_REVISION(1) void goToOffset(int offset);

 Q_SIGNALS:
  Q_REVISION(1) void changed();
  void currentIndexChanged();

 private:
  OxideQQuickNavigationHistory();

  // reimplemented from QAbstractListModel
  QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
  int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
  QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;

  QScopedPointer<OxideQQuickNavigationHistoryPrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideQQuickNavigationHistory)

#endif // OXIDE_QTQUICK_NAVIGATION_HISTORY
