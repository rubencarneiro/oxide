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

#ifndef _OXIDE_SHARED_RENDERER_PEPPER_PEPPER_FLASH_FULLSCREEN_HOST_H_
#define _OXIDE_SHARED_RENDERER_PEPPER_PEPPER_FLASH_FULLSCREEN_HOST_H_

#include "base/macros.h"
#include "ppapi/host/resource_host.h"

namespace content {
class RendererPpapiHost;
}

namespace oxide {

class PepperFlashFullscreenHost : public ppapi::host::ResourceHost {
 public:
  PepperFlashFullscreenHost(content::RendererPpapiHost* host,
                            PP_Instance instance,
                            PP_Resource resource);
  ~PepperFlashFullscreenHost() override;

 private:
  int32_t OnSetFullscreen(ppapi::host::HostMessageContext* context,
                          bool fullscreen);

  // ppapi::host::ResourceMessageHandler implementation
  int32_t OnResourceMessageReceived(
      const IPC::Message& msg,
      ppapi::host::HostMessageContext* context) override;

  content::RendererPpapiHost* host_;

  DISALLOW_COPY_AND_ASSIGN(PepperFlashFullscreenHost);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_PEPPER_PEPPER_FLASH_FULLSCREEN_HOST_H_
