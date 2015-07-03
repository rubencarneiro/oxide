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

#ifndef _OXIDE_SHARED_BROWSER_PEPPER_HOST_FACTORY_H_
#define _OXIDE_SHARED_BROWSER_PEPPER_HOST_FACTORY_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ppapi/host/host_factory.h"

namespace content {
class BrowserPpapiHost;
}

namespace oxide {

class PepperHostFactoryBrowser final : public ppapi::host::HostFactory {
 public:
  PepperHostFactoryBrowser(content::BrowserPpapiHost* host);
  ~PepperHostFactoryBrowser();

  scoped_ptr<ppapi::host::ResourceHost> CreateResourceHost(
      ppapi::host::PpapiHost* host,
      PP_Resource resource,
      PP_Instance instance,
      const IPC::Message& message) final;

 private:
  content::BrowserPpapiHost* host_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(PepperHostFactoryBrowser);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_PEPPER_HOST_FACTORY_H_
