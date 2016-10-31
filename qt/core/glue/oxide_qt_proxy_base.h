// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_GLUE_PROXY_BASE_H_
#define _OXIDE_QT_CORE_GLUE_PROXY_BASE_H_

#include <QPointer>
#include <QtGlobal>

#include "qt/core/api/oxideqglobal.h"

QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class OXIDE_QTCORE_EXPORT ProxyBasePrivate {
 public:
  ProxyBasePrivate(void* proxy);
  ~ProxyBasePrivate();

  QObject* handle() const { return handle_; }

  void SetHandle(QObject* handle);

#if defined(OXIDE_QTCORE_IMPLEMENTATION)
  static void* ProxyFromHandle(QObject* handle);
#endif

 private:
  void* proxy_;
  QPointer<QObject> handle_;
};

template <typename Impl>
class OXIDE_QTCORE_EXPORT ProxyBase {
 public:
  virtual ~ProxyBase() {}

  ProxyBase() : priv_(reinterpret_cast<void*>(this)) {}

  QObject* handle() const { return priv_.handle(); }
  void setHandle(QObject* handle) { priv_.SetHandle(handle); }

#if defined(OXIDE_QTCORE_IMPLEMENTATION)
  static Impl* FromProxyHandle(QObject* handle) {
    return reinterpret_cast<Impl*>(ProxyBasePrivate::ProxyFromHandle(handle));
  }
#endif

 private:
  ProxyBasePrivate priv_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_PROXY_BASE_H_
