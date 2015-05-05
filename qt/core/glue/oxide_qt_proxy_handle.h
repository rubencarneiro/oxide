// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_GLUE_PROXY_HANDLE_H_
#define _OXIDE_QT_CORE_GLUE_PROXY_HANDLE_H_

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>

namespace oxide {
namespace qt {

template <typename T>
class ProxyHandle {
  friend class T::ImplType;

  QScopedPointer<T> proxy_;

 public:
  virtual ~ProxyHandle() {}

 protected:
  ProxyHandle(T* proxy, QObject* q)
      : proxy_(proxy),
        q_ptr(q) {
    Q_ASSERT(proxy);
    Q_ASSERT(q);
    Q_ASSERT(!proxy->handle());
    proxy->set_handle(this);
  }

  T* proxy() const { return proxy_.data(); }

  QObject* q_ptr;
};

} // namespace qt
} // namespace oxide

#define OXIDE_Q_DECL_PROXY_HANDLE_CONVERTER(_pub, _proxy) \
 public: \
  static _pub* fromProxyHandle(_proxy* h); \
 private:

#define OXIDE_Q_IMPL_PROXY_HANDLE_CONVERTER(_pub, _proxy) \
_pub* _pub##Private::fromProxyHandle(_proxy* h) { \
  return qobject_cast<_pub*>(static_cast<_pub##Private*>(h)->q_ptr); \
}

#define OXIDE_Q_DECL_PROXY_HANDLE(_proxy) \
typedef ::oxide::qt::ProxyHandle<_proxy> _proxy##Handle; \

#define OXIDE_Q_DECL_PROXY_FOR(_impl) \
  ::oxide::qt::ProxyHandle<_impl##Proxy>* handle_; \
 public: \
  typedef _impl ImplType; \
  ::oxide::qt::ProxyHandle<_impl##Proxy>* handle() { return handle_; } \
  void set_handle(::oxide::qt::ProxyHandle<_impl##Proxy>* handle) { \
    handle_ = handle; \
  } \
 private:

#endif // _OXIDE_QT_CORE_GLUE_PROXY_HANDLE_H_
