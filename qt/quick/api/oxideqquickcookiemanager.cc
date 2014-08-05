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
#include "oxideqquickcookiemanager_p_p.h"

#include <QNetworkCookie>
#include <QtDebug>

#include "qt/core/glue/oxide_qt_web_context_adapter.h"

#include "oxideqquickwebcontext_p_p.h"

OxideQQuickCookieManagerPrivate::OxideQQuickCookieManagerPrivate(
    OxideQQuickCookieManager* q,
    OxideQQuickWebContext* webContext) :
        q_ptr(q),
        web_context_(webContext){}

void OxideQQuickCookieManagerPrivate::setCookies(
    const QList<QNetworkCookie>& cookies, QObject* callback) {
  oxide::qt::WebContextAdapter::SetCookiesRequest* request =
      new oxide::qt::WebContextAdapter::SetCookiesRequest(cookies, callback);

  OxideQQuickWebContextPrivate* adapter =
      OxideQQuickWebContextPrivate::get(web_context_);
  if (adapter) {
    adapter->doSetCookies(request);
  }
}

void OxideQQuickCookieManagerPrivate::getAllCookies(QObject* callback) {
  OxideQQuickWebContextPrivate* adapter =
      OxideQQuickWebContextPrivate::get(web_context_);
  if (adapter) {
    adapter->doGetAllCookies(callback);
  }
}

OxideQQuickCookieManagerPrivate::~OxideQQuickCookieManagerPrivate() {}

OxideQQuickCookieManager::OxideQQuickCookieManager(
    OxideQQuickWebContext* webContext,
    QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQQuickCookieManagerPrivate(this, webContext)) {}

OxideQQuickCookieManager::~OxideQQuickCookieManager() {}

void OxideQQuickCookieManager::setCookies(
    const QList<QNetworkCookie>& cookies) {
  Q_D(OxideQQuickCookieManager);

  d->setCookies(cookies, this);
}

void OxideQQuickCookieManager::getAllCookies() {
  Q_D(OxideQQuickCookieManager);

  d->getAllCookies(this);
}

#include "moc_oxideqquickcookiemanager_p.cpp"
