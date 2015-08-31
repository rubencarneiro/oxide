// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "oxide_pepper_renderer_host_factory.h"
#include "oxide_pepper_flash_renderer_host.h"
#include "oxide_pepper_flash_menu_host.h"

#include "content/public/renderer/renderer_ppapi_host.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/host/resource_host.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/proxy/ppapi_message_utils.h"
#include "ppapi/shared_impl/ppapi_permissions.h"


namespace oxide {

PepperRendererHostFactory::PepperRendererHostFactory(content::RendererPpapiHost* host)
    : host_(host) {}

PepperRendererHostFactory::~PepperRendererHostFactory()
{}

scoped_ptr<ppapi::host::ResourceHost> PepperRendererHostFactory::CreateResourceHost(
    ppapi::host::PpapiHost* host,
    PP_Resource resource,
    PP_Instance instance,
    const IPC::Message& message
) {
  DCHECK_EQ(host_->GetPpapiHost(), host);

  if (!host_->IsValidInstance(instance)) {
    return nullptr;
  }

  if (host_->GetPpapiHost()->permissions().HasPermission(
        ppapi::PERMISSION_FLASH)) {
    switch (message.type()) {
      case PpapiHostMsg_Flash_Create::ID: {
        return make_scoped_ptr(
            new PepperFlashRendererHost(host_, instance, resource));
      }
//      case PpapiHostMsg_FlashFullscreen_Create::ID: {
//       return scoped_ptr<ppapi::host::ResourceHost>(new PepperFlashFullscreenHost(
//            host_, instance, params.pp_resource()));
//      }
      case PpapiHostMsg_FlashMenu_Create::ID: {
        ppapi::proxy::SerializedFlashMenu serialized_menu;
        if (ppapi::UnpackMessage<PpapiHostMsg_FlashMenu_Create>(
          message, &serialized_menu)) {
          return make_scoped_ptr(
              new PepperFlashMenuHost(host_, instance, resource, serialized_menu));
        }
        break;
      }
      default:
        ;
    }
  }

  return scoped_ptr<ppapi::host::ResourceHost>();
}

} // oxide
