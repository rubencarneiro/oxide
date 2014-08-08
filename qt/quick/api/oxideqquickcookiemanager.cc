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

#include <climits>
#include <QList>
#include <QNetworkCookie>
#include <QtDebug>

#include "oxideqquicknetworkcookies_p.h"
#include "qt/core/glue/oxide_qt_web_context_adapter.h"

#include "oxideqquickwebcontext_p_p.h"

namespace {
int getNextRequestId()
{
  static int id = 0;
  id = id++ % INT_MAX;
  return id;
}
}

OxideQQuickCookieManagerPrivate::OxideQQuickCookieManagerPrivate(
    OxideQQuickCookieManager* q,
    OxideQQuickWebContext* webContext) :
        q_ptr(q),
        web_context_(webContext){}

int OxideQQuickCookieManagerPrivate::setCookies(
    const QString& url, const QList<QNetworkCookie>& cookies) {
  int requestId = getNextRequestId();

  OxideQQuickWebContextPrivate* adapter =
      OxideQQuickWebContextPrivate::get(web_context_);
  if (adapter) {
    adapter->doSetCookies(url, cookies, requestId);
  }
  return requestId;
}

int OxideQQuickCookieManagerPrivate::getAllCookies() {
  int requestId = getNextRequestId();

  OxideQQuickWebContextPrivate* adapter =
      OxideQQuickWebContextPrivate::get(web_context_);
  if (adapter) {
    adapter->doGetAllCookies(requestId);
  }
  return requestId;
}

OxideQQuickCookieManagerPrivate::~OxideQQuickCookieManagerPrivate() {}

OxideQQuickCookieManager::OxideQQuickCookieManager(
    OxideQQuickWebContext* webContext,
    QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQQuickCookieManagerPrivate(this, webContext)) {}

OxideQQuickCookieManager::~OxideQQuickCookieManager() {}

int OxideQQuickCookieManager::setCookies(
    const QString& url, OxideQQuickNetworkCookies* cookies) {
  Q_D(OxideQQuickCookieManager);

  if (!cookies)
    return -1;

  return d->setCookies(url, cookies->toNetworkCookies());
}

int OxideQQuickCookieManager::getAllCookies() {
  Q_D(OxideQQuickCookieManager);

  return d->getAllCookies();
}

#include "moc_oxideqquickcookiemanager_p.cpp"
