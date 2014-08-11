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

#include "oxide_pepper_host_factory_browser.h"

#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_ppapi_host.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/host/resource_host.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/shared_impl/ppapi_permissions.h"

#include "oxide_pepper_talk_host.h"
#include "oxide_pepper_flash_browser_host.h"

namespace oxide {

PepperHostFactoryBrowser::PepperHostFactoryBrowser(
    content::BrowserPpapiHost* host) :
    host_(host) {}

PepperHostFactoryBrowser::~PepperHostFactoryBrowser() {}

scoped_ptr<ppapi::host::ResourceHost>
PepperHostFactoryBrowser::CreateResourceHost(
    ppapi::host::PpapiHost* host,
    const ppapi::proxy::ResourceMessageCallParams& params,
    PP_Instance instance,
    const IPC::Message& message) {
  DCHECK(host == host_->GetPpapiHost());

  if (!host_->IsValidInstance(instance)) {
    return scoped_ptr<ppapi::host::ResourceHost>();
  }

  // Dev interfaces:
  // TODO:
  //  PpapiHostMsg_ExtensionsCommon_Create

  // Private interfaces:
  // TODO:
  //  PpapiHostMsg_Broker_Create
  if (host_->GetPpapiHost()->permissions().HasPermission(
          ppapi::PERMISSION_PRIVATE) &&
      message.type() == PpapiHostMsg_Talk_Create::ID) {
    return scoped_ptr<ppapi::host::ResourceHost>(new PepperTalkHost(
        host_, instance, params.pp_resource()));
  }

  // Flash interfaces:
  // TODO:
  //  PpapiHostMsg_Flash_Create
  //  PpapiHostMsg_FlashClipboard_Create
  //  PpapiHostMsg_FlashDRM_Create

  if (host_->GetPpapiHost()->permissions().HasPermission(
          ppapi::PERMISSION_FLASH)) {
      switch (message.type()) {
        case PpapiHostMsg_Flash_Create::ID:
          return scoped_ptr<ppapi::host::ResourceHost>(
            new PepperFlashBrowserHost(host_, instance, params.pp_resource()));
        default:
          ;
      }
  }

  return scoped_ptr<ppapi::host::ResourceHost>();
}

} // namespace oxide
