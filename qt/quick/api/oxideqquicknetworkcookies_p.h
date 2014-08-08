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

#ifndef OXIDE_Q_NETWORK_COOKIES
#define OXIDE_Q_NETWORK_COOKIES

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include <QVariant>
#include <QList>
#include <QNetworkCookie>

class OxideQQuickNetworkCookiesPrivate;

class Q_DECL_EXPORT OxideQQuickNetworkCookies : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVariant cookies READ cookies WRITE setCookies NOTIFY cookiesChanged)
  Q_PROPERTY(QStringList rawHttpCookies READ rawHttpCookies WRITE setRawHttpCookies NOTIFY rawHttpCookiesChanged)

  Q_DECLARE_PRIVATE(OxideQQuickNetworkCookies)

 public:
  OxideQQuickNetworkCookies(
      const QList<QNetworkCookie>& cookies = QList<QNetworkCookie>(),
      QObject* parent = 0);
  virtual ~OxideQQuickNetworkCookies();

  static OxideQQuickNetworkCookies* fromVariant(const QVariant& cookies);

  QVariant cookies() const;
  void setCookies(const QVariant& cookies);

  QStringList rawHttpCookies() const;
  void setRawHttpCookies(const QStringList& rawHttpCookies);

  Q_INVOKABLE QList<QNetworkCookie> toNetworkCookies() const;

 Q_SIGNALS:
  void cookiesChanged();
  void rawHttpCookiesChanged();

 private:
  QScopedPointer<OxideQQuickNetworkCookiesPrivate> d_ptr;
};

#endif // OXIDE_Q_NETWORK_COOKIES

