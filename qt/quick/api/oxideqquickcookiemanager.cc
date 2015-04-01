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

#include "oxideqquickcookiemanager_p.h"

#include <climits>
#include <QList>
#include <QNetworkCookie>
#include <QtDebug>

#include "oxideqquickwebcontext_p_p.h"

namespace {

QList<QNetworkCookie> networkCookiesFromVariantList(const QVariant& cookies) {
  if (!cookies.canConvert(QMetaType::QVariantList)) {
    return QList<QNetworkCookie>();
  }
  QList<QNetworkCookie> network_cookies;
  QList<QVariant> cl = cookies.toList();
  Q_FOREACH(QVariant cookie, cl) {
    if (!cookie.canConvert(QVariant::Map)) {
      continue;
    }
    QNetworkCookie nc;
    QVariantMap vm = cookie.toMap();
    
    nc.setName(vm.value("name").toByteArray());
    nc.setValue(vm.value("value").toByteArray());
    nc.setDomain(vm.value("domain").toString());
    nc.setPath(vm.value("path").toString());

    if (vm.contains("httponly") &&
        vm.value("httponly").canConvert(QVariant::Bool)) {
      nc.setHttpOnly(vm.value("httponly").toBool());
    }
    if (vm.contains("issecure") &&
        vm.value("issecure").canConvert(QVariant::Bool)) {
      nc.setSecure(vm.value("issecure").toBool());
    }
    if (vm.contains("expirationdate")) {
      QDateTime datetime;
      if (vm.value("expirationdate").canConvert(QVariant::DateTime)) {
        datetime = vm.value("expirationdate").toDateTime();
      }
      // ISO8601 strings will convert to QDateTime - however, it would be nice
      // to also be able to convert from RFC1123 based datetime strings
      if (!datetime.isValid() &&
          vm.value("expirationdate").canConvert(QVariant::LongLong)) {
        datetime = QDateTime::fromMSecsSinceEpoch(
            vm.value("expirationdate").toLongLong());
      }
      if (datetime.isValid()) {
        nc.setExpirationDate(datetime);
      }
    }
    network_cookies.append(nc);
  }
  return network_cookies;
}

}

class OxideQQuickCookieManagerPrivate {
 public:
  ~OxideQQuickCookieManagerPrivate() {}

 private:
  friend class OxideQQuickCookieManager;

  OxideQQuickCookieManagerPrivate(OxideQQuickWebContext* web_context)
      : web_context_(web_context) {}

  OxideQQuickWebContext* web_context_;

  Q_DISABLE_COPY(OxideQQuickCookieManagerPrivate);
};

OxideQQuickCookieManager::OxideQQuickCookieManager(
    OxideQQuickWebContext* webContext,
    QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQQuickCookieManagerPrivate(webContext)) {}

OxideQQuickCookieManager::~OxideQQuickCookieManager() {}

int OxideQQuickCookieManager::setCookies(const QUrl& url,
                                         const QVariant& cookies) {
  Q_D(OxideQQuickCookieManager);

  return OxideQQuickWebContextPrivate::get(d->web_context_)->setCookies(
      url, networkCookiesFromVariantList(cookies));
}

int OxideQQuickCookieManager::setNetworkCookies(
    const QUrl& url,
    const QList<QNetworkCookie>& cookies) {
  Q_D(OxideQQuickCookieManager);

  return OxideQQuickWebContextPrivate::get(d->web_context_)->setCookies(
      url, cookies);
}

int OxideQQuickCookieManager::getCookies(const QUrl& url) {
  Q_D(OxideQQuickCookieManager);

  return OxideQQuickWebContextPrivate::get(d->web_context_)->getCookies(url);
}

int OxideQQuickCookieManager::getAllCookies() {
  Q_D(OxideQQuickCookieManager);

  return OxideQQuickWebContextPrivate::get(d->web_context_)->getAllCookies();
}

int OxideQQuickCookieManager::deleteAllCookies() {
  Q_D(OxideQQuickCookieManager);

  return OxideQQuickWebContextPrivate::get(d->web_context_)->deleteAllCookies();
}

#include "moc_oxideqquickcookiemanager_p.cpp"
