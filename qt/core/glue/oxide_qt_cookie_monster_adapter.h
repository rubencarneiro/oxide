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

#ifndef _OXIDE_QT_CORE_GLUE_COOKIE_MONSTER_ADAPTER_H_
#define _OXIDE_QT_CORE_GLUE_COOKIE_MONSTER_ADAPTER_H_

#include <QList>
#include <QtGlobal>

#include "qt/core/glue/oxide_qt_adapter_base.h"

QT_BEGIN_NAMESPACE
class QNetworkCookie;
QT_END_NAMESPACE

namespace net {
class CookieMonster;
}

namespace oxide {
namespace qt {

class CookieMonsterAdapterPrivate;

class Q_DECL_EXPORT CookieMonsterAdapter : public AdapterBase {
 public:
  ~CookieMonsterAdapter();

  void setCookie(const QNetworkCookie& cookie);
  void getAllCookies();

  virtual void OnCookieSet(bool success) = 0;
  virtual void OnGotCookies(const QList<QNetworkCookie>& cookies) = 0;

 protected:
  CookieMonsterAdapter(QObject* q,
                       net::CookieMonster* cookieMonster);

 private:
  friend class CookieMonsterAdapterPrivate;

  QScopedPointer<CookieMonsterAdapterPrivate> priv;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_COOKIE_MONSTER_ADAPTER_H_
