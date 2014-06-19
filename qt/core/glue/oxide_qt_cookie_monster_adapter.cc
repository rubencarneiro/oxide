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

#include "oxide_qt_cookie_monster_adapter.h"
#include "oxide_qt_cookie_monster_adapter_p.h"

#include <QDateTime>
#include <QNetworkCookie>
#include <QString>

#include "base/bind.h"
#include "base/callback.h"
#include "base/time/time.h"
#include "url/gurl.h"

namespace oxide {
namespace qt {

CookieMonsterAdapterPrivate::CookieMonsterAdapterPrivate(
    CookieMonsterAdapter* adapter,
    net::CookieMonster* cookieMonster) :
    a(adapter),
    cookie_monster_(cookieMonster) {}

// static
CookieMonsterAdapterPrivate* CookieMonsterAdapterPrivate::get(
    CookieMonsterAdapter* adapter) {
  return adapter->priv.data();
}

void CookieMonsterAdapterPrivate::GotCookiesCallback(
    const net::CookieList& cookies) {
  QList<QNetworkCookie> qcookies;
  for (net::CookieList::const_iterator iter = cookies.begin();
       iter != cookies.end(); ++iter) {
    QNetworkCookie cookie;
    cookie.setName(iter->Name().c_str());
    cookie.setValue(iter->Value().c_str());
    cookie.setDomain(iter->Domain().c_str());
    cookie.setPath(iter->Path().c_str());
    cookie.setExpirationDate(QDateTime::fromMSecsSinceEpoch(
        iter->ExpiryDate().ToJsTime()));
    cookie.setSecure(iter->IsSecure());
    cookie.setHttpOnly(iter->IsHttpOnly());
    qcookies.append(cookie);
  }

  a->OnGotCookies(qcookies);
}

CookieMonsterAdapter::CookieMonsterAdapter(QObject* q,
                                           net::CookieMonster* cookieMonster) :
    AdapterBase(q),
    priv(new CookieMonsterAdapterPrivate(this, cookieMonster)) {}

CookieMonsterAdapter::~CookieMonsterAdapter() {}

void CookieMonsterAdapter::setCookie(const QNetworkCookie& cookie) {
  if (!priv->cookie_monster_) {
    OnCookieSet(false);
    return;
  }
  priv->cookie_monster_->SetCookieWithDetailsAsync(GURL(),
    std::string(cookie.name().constData()),
    std::string(cookie.value().constData()),
    std::string(cookie.domain().toUtf8().constData()),
    std::string(cookie.path().toUtf8().constData()),
    base::Time::FromJsTime(cookie.expirationDate().toMSecsSinceEpoch()),
    cookie.isSecure(),
    cookie.isHttpOnly(),
    net::COOKIE_PRIORITY_DEFAULT,
    base::Bind(&CookieMonsterAdapter::OnCookieSet, base::Unretained(this)));
}

void CookieMonsterAdapter::getAllCookies() {
  if (!priv->cookie_monster_) {
    OnGotCookies(QList<QNetworkCookie>());
    return;
  }
  priv->cookie_monster_->GetAllCookiesAsync(
    base::Bind(&CookieMonsterAdapterPrivate::GotCookiesCallback,
               base::Unretained(priv.data())));
}

} // namespace qt
} // namespace oxide
