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

#ifndef OXIDE_QTQUICK_NAVIGATION_ITEM
#define OXIDE_QTQUICK_NAVIGATION_ITEM

#include <QtCore/QDateTime>
#include <QtCore/QMetaType>
#include <QtCore/QScopedPointer>
#include <QtCore/QString>
#include <QtCore/QtGlobal>
#include <QtCore/QUrl>

#include <OxideQtQuick/oxideqquickglobal.h>

class OxideQQuickNavigationHistoryPrivate;
class OxideQQuickNavigationItemData;

class OXIDE_QTQUICK_EXPORT OxideQQuickNavigationItem {
  Q_GADGET
  Q_PROPERTY(QUrl url READ url)
  Q_PROPERTY(QUrl originalUrl READ originalUrl)
  Q_PROPERTY(QString title READ title)
  Q_PROPERTY(QDateTime timestamp READ timestamp)

 public:
  OxideQQuickNavigationItem();
  OxideQQuickNavigationItem(const OxideQQuickNavigationItem& other);

  ~OxideQQuickNavigationItem();

  QUrl url() const;
  QUrl originalUrl() const;

  QString title() const;

  QDateTime timestamp() const;

  OxideQQuickNavigationItem& operator=(const OxideQQuickNavigationItem& other);
  bool operator==(const OxideQQuickNavigationItem& other) const;
  bool operator!=(const OxideQQuickNavigationItem& other) const;

 private:
  friend class OxideQQuickNavigationHistoryPrivate;
  friend class OxideQQuickNavigationItemData;

  Q_DECL_HIDDEN OxideQQuickNavigationItem(OxideQQuickNavigationItemData& d);

  QScopedPointer<OxideQQuickNavigationItemData> d;
};

Q_DECLARE_METATYPE(OxideQQuickNavigationItem);

#endif // OXIDE_QTQUICK_NAVIGATION_ITEM
