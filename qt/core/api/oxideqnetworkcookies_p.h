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

#ifndef _OXIDE_QT_CORE_API_NETWORK_COOKIES_P_H_
#define _OXIDE_QT_CORE_API_NETWORK_COOKIES_P_H_

#include <QString>
#include <QtGlobal>
#include <QUrl>
#include <QNetworkCookie>
#include <QVariant>
#include <QStringList>

class OxideQQuickNetworkCookiesPrivate {
 public:
  OxideQQuickNetworkCookiesPrivate(
      const QList<QNetworkCookie>& cookies);

  QVariant cookies() const;
  void setCookies(const QVariant& cookies);

  QStringList rawHttpCookies() const;
  void setRawHttpCookies(const QStringList& rawHttpCookies);

  QList<QNetworkCookie> toNetworkCookies() const;

 private:

  QList<QNetworkCookie> cookies_;
};

#endif // _OXIDE_QT_CORE_API_NETWORK_COOKIES_P_H_
