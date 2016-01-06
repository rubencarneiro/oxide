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

// Some of this file is based on
// chrome/renderer/pepper/pepper_flash_font_file_host.cc from Chromium:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "oxide_pepper_flash_font_file_host.h"

#include <string>

#include "content/public/common/child_process_sandbox_support_linux.h"
#include "content/public/renderer/renderer_ppapi_host.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/c/trusted/ppb_browser_font_trusted.h"
#include "ppapi/host/dispatch_host_message.h"
#include "ppapi/host/host_message_context.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/proxy/serialized_structs.h"

namespace oxide {

int32_t PepperFlashFontFileHost::OnGetFontTable(
    ppapi::host::HostMessageContext* context,
    uint32_t table) {
  std::string contents;
  int32_t result = PP_ERROR_FAILED;
  int fd = fd_.get();
  if (fd != -1) {
    size_t length = 0;
    if (content::GetFontTable(fd, table, 0 /* offset */, NULL, &length)) {
      contents.resize(length);
      uint8_t* contents_ptr =
          reinterpret_cast<uint8_t*>(const_cast<char*>(contents.c_str()));
      if (content::GetFontTable(
              fd, table, 0 /* offset */, contents_ptr, &length)) {
        result = PP_OK;
      } else {
        contents.clear();
      }
    }
  }

  context->reply_msg = PpapiPluginMsg_FlashFontFile_GetFontTableReply(contents);
  return result;
}

int32_t PepperFlashFontFileHost::OnResourceMessageReceived(
    const IPC::Message& msg,
    ppapi::host::HostMessageContext* context) {
  PPAPI_BEGIN_MESSAGE_MAP(PepperFlashFontFileHost, msg)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL(PpapiHostMsg_FlashFontFile_GetFontTable,
                                      OnGetFontTable)
  PPAPI_END_MESSAGE_MAP()
  return PP_ERROR_FAILED;
}

PepperFlashFontFileHost::PepperFlashFontFileHost(
    content::RendererPpapiHost* host,
    PP_Instance instance,
    PP_Resource resource,
    const ppapi::proxy::SerializedFontDescription& description,
    PP_PrivateFontCharset charset)
    : ppapi::host::ResourceHost(host->GetPpapiHost(), instance, resource) {
  fd_.reset(content::MatchFontWithFallback(
      description.face,
      description.weight >= PP_BROWSERFONT_TRUSTED_WEIGHT_BOLD,
      description.italic,
      charset,
      PP_BROWSERFONT_TRUSTED_FAMILY_DEFAULT));
}

PepperFlashFontFileHost::~PepperFlashFontFileHost() {}

} // namespace oxide
