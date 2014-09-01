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
#include <QNetworkCookie>
#include <QtGlobal>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

class OxideQQuickCookieManagerPrivate;
class OxideQQuickWebContext;

class OxideQQuickCookieManager : public QObject {
  Q_OBJECT

  Q_ENUMS(RequestStatus)

  Q_DECLARE_PRIVATE(OxideQQuickCookieManager)
  Q_DISABLE_COPY(OxideQQuickCookieManager)

public:

  enum RequestStatus {
    RequestStatusOK,
    RequestStatusError
  };

  OxideQQuickCookieManager(OxideQQuickWebContext* webContext,
                           QObject* parent = NULL);
  virtual ~OxideQQuickCookieManager();

  Q_INVOKABLE int setCookies(const QUrl& url, const QVariant& cookies);
  Q_INVOKABLE int getCookies(const QUrl& url);
  Q_INVOKABLE int getAllCookies();

Q_SIGNALS:
  void cookiesSet(int requestId, RequestStatus status);
  void gotCookies(int requestId, const QVariant& cookies);

private:
  QScopedPointer<OxideQQuickCookieManagerPrivate> d_ptr;
};

#endif // _OXIDE_QT_QUICK_API_COOKIE_MANAGER_P_H_
