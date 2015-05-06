// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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


#ifndef _OXIDE_SHARED_PEPPER_FLASH_RENDERER_HOST_H_
#define _OXIDE_SHARED_PEPPER_FLASH_RENDERER_HOST_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"
#include "ppapi/host/host_message_context.h"
#include "ppapi/host/resource_host.h"

struct PP_Rect;

namespace ppapi {
struct URLRequestInfoData;
}

namespace ppapi {
namespace proxy {
struct PPBFlash_DrawGlyphs_Params;
}
}

namespace content {
class RendererPpapiHost;
}


namespace oxide {

class PepperFlashRendererHost final : public ppapi::host::ResourceHost {
 public:
  PepperFlashRendererHost(content::RendererPpapiHost* host,
                          PP_Instance instance,
                          PP_Resource resource);
  ~PepperFlashRendererHost();

  int32_t OnResourceMessageReceived(const IPC::Message& msg,
      ppapi::host::HostMessageContext* context) override;

 private:
  int32_t OnGetProxyForURL(ppapi::host::HostMessageContext* host_context,
                           const std::string& url);
  int32_t OnSetInstanceAlwaysOnTop(
      ppapi::host::HostMessageContext* host_context,
      bool on_top);
  int32_t OnDrawGlyphs(ppapi::host::HostMessageContext* host_context,
                       ppapi::proxy::PPBFlash_DrawGlyphs_Params params);
  int32_t OnNavigate(ppapi::host::HostMessageContext* host_context,
                     const ppapi::URLRequestInfoData& data,
                     const std::string& target,
                     bool from_user_action);
  int32_t OnIsRectTopmost(ppapi::host::HostMessageContext* host_context,
                          const PP_Rect& rect);
  int32_t OnInvokePrinting(ppapi::host::HostMessageContext* host_context);

  content::RendererPpapiHost* host_;
  base::WeakPtrFactory<PepperFlashRendererHost> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(PepperFlashRendererHost);
};

} // oxide

#endif // _OXIDE_SHARED_PEPPER_FLASH_RENDERER_HOST_H_

