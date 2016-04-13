// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "oxide_pepper_renderer_host_factory.h"

#include <memory>

#include "base/memory/ptr_util.h"
#include "content/public/renderer/renderer_ppapi_host.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/host/resource_host.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/proxy/ppapi_message_utils.h"
#include "ppapi/shared_impl/ppapi_permissions.h"

#include "oxide_pepper_flash_font_file_host.h"
#include "oxide_pepper_flash_fullscreen_host.h"
#include "oxide_pepper_flash_renderer_host.h"
#include "oxide_pepper_flash_menu_host.h"

namespace oxide {

PepperRendererHostFactory::PepperRendererHostFactory(
    content::RendererPpapiHost* host)
    : host_(host) {}

PepperRendererHostFactory::~PepperRendererHostFactory() {}

std::unique_ptr<ppapi::host::ResourceHost>
PepperRendererHostFactory::CreateResourceHost(
    ppapi::host::PpapiHost* host,
    PP_Resource resource,
    PP_Instance instance,
    const IPC::Message& message) {
  DCHECK_EQ(host_->GetPpapiHost(), host);

  if (!host_->IsValidInstance(instance)) {
    return nullptr;
  }

  if (host_->GetPpapiHost()->permissions().HasPermission(
        ppapi::PERMISSION_FLASH)) {
    switch (message.type()) {
      case PpapiHostMsg_Flash_Create::ID:
        return base::WrapUnique(
            new PepperFlashRendererHost(host_, instance, resource));

      case PpapiHostMsg_FlashFontFile_Create::ID: {
        ppapi::proxy::SerializedFontDescription description;
        PP_PrivateFontCharset charset;
        if (ppapi::UnpackMessage<PpapiHostMsg_FlashFontFile_Create>(
                message, &description, &charset)) {
          return base::WrapUnique(
              new PepperFlashFontFileHost(
                host_, instance, resource, description, charset));
        }
        break;
      }
      case PpapiHostMsg_FlashFullscreen_Create::ID:
        return base::WrapUnique(
            new PepperFlashFullscreenHost(host_, instance, resource));

      case PpapiHostMsg_FlashMenu_Create::ID: {
        ppapi::proxy::SerializedFlashMenu serialized_menu;
        if (ppapi::UnpackMessage<PpapiHostMsg_FlashMenu_Create>(
          message, &serialized_menu)) {
          return base::WrapUnique(
              new PepperFlashMenuHost(host_,
                                      instance,
                                      resource,
                                      serialized_menu));
        }
        break;
      }
      default:
        ;
    }
  }

  return nullptr;
}

} // oxide
