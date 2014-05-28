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

#include "oxide_pepper_talk_host.h"

#include "content/public/browser/browser_ppapi_host.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/host/dispatch_host_message.h"
#include "ppapi/host/host_message_context.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/proxy/ppapi_messages.h"

namespace oxide {

int32_t PepperTalkHost::OnResourceMessageReceived(
    const IPC::Message& msg,
    ppapi::host::HostMessageContext* context) {
  IPC_BEGIN_MESSAGE_MAP(PepperTalkHost, msg)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL(PpapiHostMsg_Talk_RequestPermission,
                                      OnRequestPermission)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL_0(PpapiHostMsg_Talk_StartRemoting,
                                        OnStartRemoting)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL_0(PpapiHostMsg_Talk_StopRemoting,
                                        OnStopRemoting)
  IPC_END_MESSAGE_MAP()
  return PP_ERROR_FAILED;
}

int32_t PepperTalkHost::OnRequestPermission(
    ppapi::host::HostMessageContext* context,
    PP_TalkPermission permission) {
  return PP_ERROR_NOTSUPPORTED;
}

int32_t PepperTalkHost::OnStartRemoting(
    ppapi::host::HostMessageContext* context) {
  return PP_ERROR_NOTSUPPORTED;
}

int32_t PepperTalkHost::OnStopRemoting(
    ppapi::host::HostMessageContext* context) {
  return PP_OK;
}

PepperTalkHost::PepperTalkHost(content::BrowserPpapiHost* host,
                               PP_Instance instance,
                               PP_Resource resource) :
    ppapi::host::ResourceHost(host->GetPpapiHost(), instance, resource) {}

PepperTalkHost::~PepperTalkHost() {}

} // namespace oxide
