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

#ifndef _OXIDE_SHARED_RENDERER_PEPPER_PEPPER_FLASH_FONT_FILE_HOST_H_
#define _OXIDE_SHARED_RENDERER_PEPPER_PEPPER_FLASH_FONT_FILE_HOST_H_

#include "base/files/scoped_file.h"
#include "base/macros.h"
#include "ppapi/c/private/pp_private_font_charset.h"
#include "ppapi/host/resource_host.h"

namespace content {
class RendererPpapiHost;
}

namespace ppapi {
namespace host {
class HostMessageContext;
}
namespace proxy {
struct SerializedFontDescription;
}
}

namespace oxide {

class PepperFlashFontFileHost : public ppapi::host::ResourceHost {
 public:
  PepperFlashFontFileHost(
      content::RendererPpapiHost* host,
      PP_Instance instance,
      PP_Resource resouce,
      const ppapi::proxy::SerializedFontDescription& description,
      PP_PrivateFontCharset charset);
  ~PepperFlashFontFileHost() override;

 private:
  int32_t OnGetFontTable(ppapi::host::HostMessageContext* context,
                         uint32_t table);

  // ppapi::host::ResourceMessageHandler implementation
  int32_t OnResourceMessageReceived(
      const IPC::Message& msg,
      ppapi::host::HostMessageContext* context) override;

  base::ScopedFD fd_;

  DISALLOW_COPY_AND_ASSIGN(PepperFlashFontFileHost);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_PEPPER_PEPPER_FLASH_FONT_FILE_HOST_H_
