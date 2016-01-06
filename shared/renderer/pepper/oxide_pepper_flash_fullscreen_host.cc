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

#include "oxide_pepper_flash_fullscreen_host.h"

#include "content/public/renderer/pepper_plugin_instance.h"
#include "content/public/renderer/renderer_ppapi_host.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/host/dispatch_host_message.h"
#include "ppapi/proxy/ppapi_messages.h"

namespace oxide {

int32_t PepperFlashFullscreenHost::OnSetFullscreen(
    ppapi::host::HostMessageContext* context,
    bool fullscreen) {
  content::PepperPluginInstance* plugin_instance =
      host_->GetPluginInstance(pp_instance());
  if (!plugin_instance) {
    return PP_ERROR_FAILED;
  }

  if (plugin_instance->FlashSetFullscreen(fullscreen, true)) {
    return PP_OK;
  }

  return PP_ERROR_FAILED;
}

int32_t PepperFlashFullscreenHost::OnResourceMessageReceived(
    const IPC::Message& msg,
    ppapi::host::HostMessageContext* context) {
  PPAPI_BEGIN_MESSAGE_MAP(PepperFlashFullscreenHost, msg)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL(
        PpapiHostMsg_FlashFullscreen_SetFullscreen, OnSetFullscreen)
  PPAPI_END_MESSAGE_MAP()
  return PP_ERROR_FAILED;
}

PepperFlashFullscreenHost::PepperFlashFullscreenHost(
    content::RendererPpapiHost* host,
    PP_Instance instance,
    PP_Resource resource)
    : ppapi::host::ResourceHost(host->GetPpapiHost(), instance, resource),
      host_(host) {}

PepperFlashFullscreenHost::~PepperFlashFullscreenHost() {}

} // namespace oxide
