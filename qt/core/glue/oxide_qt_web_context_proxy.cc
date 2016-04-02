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

#include "oxide_qt_web_context_proxy.h"

#include "net/base/ip_address_number.h"

#include "qt/core/browser/oxide_qt_web_context.h"
#include "shared/browser/oxide_devtools_manager.h"

namespace oxide {
namespace qt {

// static
WebContextProxy* WebContextProxy::create(WebContextProxyClient* client,
                                         QObject* handle) {
  return new WebContext(client, handle);
}

// static
void WebContextProxy::getValidDevtoolsPorts(int* min, int* max) {
  oxide::DevToolsManager::GetValidPorts(min, max);
}

// static
bool WebContextProxy::checkIPAddress(const QString& address) {
  net::IPAddressNumber unused;
  return net::ParseIPLiteralToNumber(address.toStdString(), &unused);
}

WebContextProxy::~WebContextProxy() {}

} // namespace qt
} // namespace oxide
