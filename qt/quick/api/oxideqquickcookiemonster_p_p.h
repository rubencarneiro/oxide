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

#ifndef _OXIDE_QT_QUICK_API_COOKIE_MONSTER_P_P_H_
#define _OXIDE_QT_QUICK_API_COOKIE_MONSTER_P_P_H_

#include <QObject>
#include <QtGlobal>

#include "qt/core/glue/oxide_qt_cookie_monster_adapter.h"

#include "qt/quick/api/oxideqquickcookiemonster_p.h"

class OxideQQuickCookieMonsterPrivate Q_DECL_FINAL :
    public QObject,
    public oxide::qt::CookieMonsterAdapter {
  Q_OBJECT
  Q_DECLARE_PUBLIC(OxideQQuickCookieMonster)

 public:
  ~OxideQQuickCookieMonsterPrivate();

  void setCookies(const QList<QNetworkCookie>& cookie);

 private:
  OxideQQuickCookieMonsterPrivate(OxideQQuickCookieMonster* q,
                                  net::CookieMonster* cookieMonster);

  virtual void OnCookieSet(bool success);
  virtual void OnGotCookies(const QList<QNetworkCookie>& cookies);

  int cookie_set_pending_calls_;
  bool cookie_set_result_;

  Q_DISABLE_COPY(OxideQQuickCookieMonsterPrivate);
};

#endif // _OXIDE_QT_QUICK_API_COOKIE_MONSTER_P_P_H_
