// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#ifndef _OXIDE_QT_QUICK_API_COOKIE_MANAGER_P_H_
#define _OXIDE_QT_QUICK_API_COOKIE_MANAGER_P_H_

#include <QList>
#include <QObject>
#include <QtGlobal>
#include <QVariant>

class OxideQQuickWebContext;

QT_USE_NAMESPACE

class OxideQQuickNetworkCookies;
class OxideQQuickCookieManagerPrivate;

class OxideQQuickCookieManager : public QObject {
  Q_OBJECT

  Q_DECLARE_PRIVATE(OxideQQuickCookieManager)
  Q_DISABLE_COPY(OxideQQuickCookieManager)

public:
  OxideQQuickCookieManager(OxideQQuickWebContext* webContext,
                           QObject* parent = NULL);
  virtual ~OxideQQuickCookieManager();

  Q_INVOKABLE int setCookies(const QString& url, const QVariant& cookies);
  Q_INVOKABLE int getAllCookies();

Q_SIGNALS:
  void cookiesSet(int requestId, bool success);
  void gotCookies(int requestId, OxideQQuickNetworkCookies* cookies);

private:
  QScopedPointer<OxideQQuickCookieManagerPrivate> d_ptr;
};

#endif // _OXIDE_QT_QUICK_API_COOKIE_MANAGER_P_H_
