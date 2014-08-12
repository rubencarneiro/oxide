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

#ifndef _OXIDE_QT_QUICK_API_COOKIE_MANAGER_P_P_H_
#define _OXIDE_QT_QUICK_API_COOKIE_MANAGER_P_P_H_

#include <QObject>
#include <QtGlobal>
#include <QList>
#include <QNetworkCookie>

#include "qt/quick/api/oxideqquickcookiemanager_p.h"

class OxideQQuickCookieManagerPrivate Q_DECL_FINAL :
    public QObject {
  Q_OBJECT
  Q_DECLARE_PUBLIC(OxideQQuickCookieManager)

 public:
  ~OxideQQuickCookieManagerPrivate();

  int setCookies(const QString& url, const QList<QNetworkCookie>& cookie);
  int getAllCookies();

 private:
  OxideQQuickCookieManagerPrivate(OxideQQuickCookieManager* q,
                                  OxideQQuickWebContext* webContext);

  OxideQQuickWebContext* web_context_;

  OxideQQuickCookieManager* q_ptr;
  Q_DISABLE_COPY(OxideQQuickCookieManagerPrivate);
};

#endif // _OXIDE_QT_QUICK_API_COOKIE_MANAGER_P_P_H_
