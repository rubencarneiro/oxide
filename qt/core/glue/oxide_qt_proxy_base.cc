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

#include "oxide_qt_proxy_base.h"

#include <QObject>
#include <QVariant>

#include "base/logging.h"

namespace oxide {
namespace qt {

namespace {
const char g_ProxyPropertyName[] = "_oxide_PrivateProxyObject";
}

// static
void* ProxyBasePrivate::ProxyFromHandle(QObject* handle) {
  if (!handle) {
    return nullptr;
  }

  return handle->property(g_ProxyPropertyName).value<void*>();
}

ProxyBasePrivate::ProxyBasePrivate(void* proxy)
    : proxy_(proxy) {
  DCHECK(proxy);
}

ProxyBasePrivate::~ProxyBasePrivate() {}

void ProxyBasePrivate::SetHandle(QObject* handle) {
  if (handle == handle_) {
    return;
  }
  if (handle_) {
    DCHECK_EQ(ProxyFromHandle(handle_), proxy_);
    handle_->setProperty(g_ProxyPropertyName, QVariant());
  }
  handle_ = handle;
  if (handle_) {
    DCHECK(!ProxyFromHandle(handle_));
    handle_->setProperty(g_ProxyPropertyName, QVariant::fromValue(proxy_));
  }
}

} // namespace qt
} // namespace oxide
