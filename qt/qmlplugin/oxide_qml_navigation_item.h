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

#ifndef _OXIDE_QMLPLUGIN_NAVIGATION_ITEM_H_
#define _OXIDE_QMLPLUGIN_NAVIGATION_ITEM_H_

#include <QDateTime>
#include <QString>
#include <QtGlobal>
#include <QtQml/private/qqmlvaluetype_p.h>
#include <QUrl>

#include "qt/quick/api/oxideqquicknavigationitem.h"

namespace oxide {
namespace qmlplugin {

class NavigationItem : public QQmlValueTypeBase<OxideQQuickNavigationItem> {
  Q_OBJECT
  Q_PROPERTY(QUrl url READ url)
  Q_PROPERTY(QUrl originalUrl READ originalUrl)
  Q_PROPERTY(QString title READ title)
  Q_PROPERTY(QDateTime timestamp READ timestamp)

 public:
  NavigationItem(QObject* parent = nullptr);
  ~NavigationItem() override;

  QUrl url() const;
  QUrl originalUrl() const;
  QString title() const;
  QDateTime timestamp() const;

  // QQmlValueType implementation
  QString toString() const override;
  bool isEqual(const QVariant& other) const override;
};

} // namespace qmlplugin
} // namespace oxide

#endif // _OXIDE_QMLPLUGIN_NAVIGATION_ITEM_H_
