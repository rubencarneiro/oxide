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

#include "oxideqquickcookiemonster_p.h"
#include "oxideqquickcookiemonster_p_p.h"

#include <QNetworkCookie>
#include <QtDebug>

#include "qt/core/glue/oxide_qt_cookie_monster_adapter.h"

OxideQQuickCookieMonsterPrivate::OxideQQuickCookieMonsterPrivate(
    OxideQQuickCookieMonster* q,
    net::CookieMonster* cookieMonster) :
    oxide::qt::CookieMonsterAdapter(
        q,
        cookieMonster),
    cookie_set_pending_calls_(0),
    cookie_set_result_(true) {}

void OxideQQuickCookieMonsterPrivate::setCookies(
    const QList<QNetworkCookie>& cookies) {
  cookie_set_pending_calls_ += cookies.count();
  Q_FOREACH(const QNetworkCookie& cookie, cookies) {
    setCookie(cookie);
  }
}

void OxideQQuickCookieMonsterPrivate::OnCookieSet(bool success) {
  Q_Q(OxideQQuickCookieMonster);
  Q_ASSERT(cookie_set_pending_calls_ > 0);

  cookie_set_pending_calls_--;

  if (!success) {
    cookie_set_result_ = false;
  }

  if (cookie_set_pending_calls_ == 0) {
    Q_EMIT q->cookiesSet(cookie_set_result_);
    cookie_set_result_ = true;
  }
}

void OxideQQuickCookieMonsterPrivate::OnGotCookies(
    const QList<QNetworkCookie>& cookies) {
  Q_Q(OxideQQuickCookieMonster);

  Q_EMIT q->gotCookies(cookies);
}

OxideQQuickCookieMonsterPrivate::~OxideQQuickCookieMonsterPrivate() {}

OxideQQuickCookieMonster::OxideQQuickCookieMonster(
    net::CookieMonster* cookieMonster,
    QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQQuickCookieMonsterPrivate(this, cookieMonster)) {}

OxideQQuickCookieMonster::~OxideQQuickCookieMonster() {}

void OxideQQuickCookieMonster::setCookies(
    const QList<QNetworkCookie>& cookies) {
  Q_D(OxideQQuickCookieMonster);

  d->setCookies(cookies);
}

void OxideQQuickCookieMonster::getAllCookies() {
  Q_D(OxideQQuickCookieMonster);

  d->getAllCookies();
}

#include "moc_oxideqquickcookiemonster_p.cpp"
