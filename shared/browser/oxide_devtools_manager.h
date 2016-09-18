// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_DEVTOOLS_MANAGER_H_
#define _OXIDE_SHARED_BROWSER_DEVTOOLS_MANAGER_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "components/keyed_service/core/keyed_service.h"

#include "shared/common/oxide_shared_export.h"

namespace content {
class BrowserContext;
class DevToolsHttpHandler;
}

namespace oxide {

class BrowserContext;
class DevToolsManagerFactory;

// XXX(chrisccoulson): This per-BrowserContext class is probably going to go
//  away - see https://launchpad.net/bugs/1622247
class OXIDE_SHARED_EXPORT DevToolsManager : public KeyedService {
 public:
  static DevToolsManager* Get(content::BrowserContext* context);

  bool enabled() const { return enabled_; }
  int port() const { return port_; }
  std::string address() const { return address_; }

  void SetEnabled(bool enabled);
  void SetPort(int port);
  void SetAddress(const std::string& ip_literal);

  static void GetValidPorts(int* min, int* max);

 private:
  friend class DevToolsManagerFactory;

  DevToolsManager(BrowserContext* context);
  ~DevToolsManager() override;

  BrowserContext* context_;

  bool enabled_;
  int port_;
  std::string address_;

  std::unique_ptr<content::DevToolsHttpHandler> http_handler_;

  DISALLOW_COPY_AND_ASSIGN(DevToolsManager);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_DEVTOOLS_MANAGER_H_
