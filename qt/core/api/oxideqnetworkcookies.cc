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

#include "oxideqnetworkcookies.h"
#include "oxideqnetworkcookies_p.h"

#include <QDateTime>
#include <QDebug>

namespace {
QList<QNetworkCookie> networkCookiesFromVariantList(const QVariant& cookies) {
  if (!cookies.canConvert(QMetaType::QVariantList)) {
    return QList<QNetworkCookie>();
  }
  QList<QNetworkCookie> networkCookies;
  QList<QVariant> cl = cookies.toList();
  Q_FOREACH(QVariant cookie, cl) {
    if (!cookie.canConvert(QVariant::Map)) {
      continue;
    }
    QNetworkCookie nc;
    QVariantMap vm = cookie.toMap();
    
    if (!vm.contains("name") || !vm.contains("value")) {
      continue;
    }
    
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
    if (vm.contains("expirationdate") &&
	vm.value("expirationdate").canConvert(QVariant::LongLong)) {
      bool ok = false;
      qlonglong date = vm.value("expirationdate").toLongLong(&ok);
      if (ok)
	nc.setExpirationDate(QDateTime::fromMSecsSinceEpoch(date));
    }
    networkCookies.append(nc);
  }
  return networkCookies;
}
}

OxideQQuickNetworkCookiesPrivate::OxideQQuickNetworkCookiesPrivate(
      const QList<QNetworkCookie>& cookies)
  : cookies_(cookies) {
}

QList<QNetworkCookie> OxideQQuickNetworkCookiesPrivate::toNetworkCookies() const {
  return cookies_;
}

QVariant OxideQQuickNetworkCookiesPrivate::cookies() const {
  QList<QVariant> cookieMapList;
  Q_FOREACH(QNetworkCookie cookie, cookies_) {
    QVariantMap cm;
    cm.insert("name", QVariant(cookie.name()));
    cm.insert("value", QVariant(cookie.value()));
    cm.insert("domain", QVariant(cookie.domain()));
    cm.insert("path", QVariant(cookie.path()));
    cm.insert("httponly", QVariant(cookie.isHttpOnly()));
    cm.insert("issecure", QVariant(cookie.isSecure()));
    cm.insert("issessioncookie", QVariant(cookie.isSessionCookie()));
    cm.insert("expirationdate", QVariant(cookie.expirationDate()));
    cookieMapList.append(cm);
  }
  return QVariant(cookieMapList);
}

void OxideQQuickNetworkCookiesPrivate::setCookies(const QVariant& cookies) {
  if (!cookies.canConvert(QMetaType::QVariantList)) {
    return;
  }
  cookies_ = networkCookiesFromVariantList(cookies);
}

QStringList OxideQQuickNetworkCookiesPrivate::rawHttpCookies() const {
  QStringList rawCookies;
  Q_FOREACH(QNetworkCookie cookie, cookies_) {
    rawCookies.append(QString(cookie.toRawForm()));
  }
  return rawCookies;
}

void OxideQQuickNetworkCookiesPrivate::setRawHttpCookies(const QStringList& rawHttpCookies) {
  cookies_.clear();
  Q_FOREACH(QString rawHttpCookie, rawHttpCookies) {
    QList<QNetworkCookie> cookies = QNetworkCookie::parseCookies(rawHttpCookie.toUtf8());
    if (cookies.count()) {
      cookies_.append(cookies);
    }
  }
}

OxideQQuickNetworkCookies::OxideQQuickNetworkCookies(
      const QList<QNetworkCookie>& cookies,
      QObject* parent)
          : QObject(parent),
	    d_ptr(new OxideQQuickNetworkCookiesPrivate(cookies)) {
}

OxideQQuickNetworkCookies::~OxideQQuickNetworkCookies() {
}

QVariant OxideQQuickNetworkCookies::cookies() const {
  Q_D(const OxideQQuickNetworkCookies);
  return d->cookies();
}

void OxideQQuickNetworkCookies::setCookies(const QVariant& cookies) {
  Q_D(OxideQQuickNetworkCookies);
  d->setCookies(cookies);
  emit cookiesChanged();
}

QStringList OxideQQuickNetworkCookies::rawHttpCookies() const {
  Q_D(const OxideQQuickNetworkCookies);
  return d->rawHttpCookies();
}

void OxideQQuickNetworkCookies::setRawHttpCookies(const QStringList& rawHttpCookies) {
  Q_D(OxideQQuickNetworkCookies);
  d->setRawHttpCookies(rawHttpCookies);
  emit rawHttpCookiesChanged();
}

QList<QNetworkCookie> OxideQQuickNetworkCookies::toNetworkCookies() const {
  Q_D(const OxideQQuickNetworkCookies);
  return d->toNetworkCookies();
}

// static
OxideQQuickNetworkCookies* OxideQQuickNetworkCookies::fromVariant(
      const QVariant& cookies) {
  return new OxideQQuickNetworkCookies(networkCookiesFromVariantList(cookies));
}
