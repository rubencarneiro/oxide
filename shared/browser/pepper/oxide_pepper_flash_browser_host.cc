// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "oxide_pepper_flash_browser_host.h"

#include "base/logging.h"
#include "base/time/time.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/browser_ppapi_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/cookie_store_factory.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/host/dispatch_host_message.h"
#include "ppapi/host/host_message_context.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/shared_impl/time_conversion.h"
#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_browser_context_delegate.h"

namespace oxide {

namespace {

// Do work on the UI thread
int32_t GetRestrictions(int render_process_id,
    const GURL& document_url,
    const GURL& plugin_url
) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  int32_t restrictions = PP_FLASHLSORESTRICTIONS_NONE;
  if (!document_url.is_valid()) {
    return restrictions;
  }

  BrowserContextIOData* io_data = nullptr;
  content::RenderProcessHost* render_process_host =
      content::RenderProcessHost::FromID(render_process_id);
  if (render_process_host) {
    io_data = BrowserContextIOData::FromResourceContext(
      render_process_host->GetBrowserContext()->GetResourceContext());
  }

  if (!io_data) {
    return restrictions;
  }

  // TODO(chrisccoulson): Implement PP_FLASHLSORESTRICTIONS_IN_MEMORY when
  // we have content settings

  if (io_data->CanAccessCookies(
      document_url,
      plugin_url,
      false) != STORAGE_PERMISSION_ALLOW) {
      restrictions = PP_FLASHLSORESTRICTIONS_BLOCK;
  }

  return restrictions;
}

}  // namespace


PepperFlashBrowserHost::PepperFlashBrowserHost(content::BrowserPpapiHost* host,
                                               PP_Instance instance,
                                               PP_Resource resource)
    : ResourceHost(host->GetPpapiHost(), instance, resource),
      host_(host),
      weak_factory_(this) {
  int unused;
  host->GetRenderFrameIDsForInstance(instance, &render_process_id_, &unused);
}

PepperFlashBrowserHost::~PepperFlashBrowserHost() {}

int32_t PepperFlashBrowserHost::OnResourceMessageReceived(
    const IPC::Message& msg,
    ppapi::host::HostMessageContext* context) {

  PPAPI_BEGIN_MESSAGE_MAP(PepperFlashBrowserHost, msg)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL_0(PpapiHostMsg_Flash_UpdateActivity,
                                        OnUpdateActivity)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL(PpapiHostMsg_Flash_GetLocalTimeZoneOffset,
                                      OnGetLocalTimeZoneOffset)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL_0(
        PpapiHostMsg_Flash_GetLocalDataRestrictions, OnGetLocalDataRestrictions)
  PPAPI_END_MESSAGE_MAP()

  return PP_ERROR_FAILED;
}

int32_t PepperFlashBrowserHost::OnUpdateActivity(
    ppapi::host::HostMessageContext* host_context) {
  return PP_OK;
}

int32_t PepperFlashBrowserHost::OnGetLocalTimeZoneOffset(
    ppapi::host::HostMessageContext* host_context,
    const base::Time& t) {

  // The reason for this processing being in the browser process is that on
  // Linux, the localtime calls require filesystem access prohibited by the
  // sandbox.

  host_context->reply_msg = PpapiPluginMsg_Flash_GetLocalTimeZoneOffsetReply(
    ppapi::PPGetLocalTimeZoneOffset(t));

  return PP_OK;
}

int32_t PepperFlashBrowserHost::OnGetLocalDataRestrictions(
    ppapi::host::HostMessageContext* context) {

  GURL document_url = host_->GetDocumentURLForInstance(pp_instance());
  GURL plugin_url = host_->GetPluginURLForInstance(pp_instance());

  content::BrowserThread::PostTaskAndReplyWithResult(
    content::BrowserThread::UI,
    FROM_HERE,
    base::Bind(&GetRestrictions, render_process_id_, document_url, plugin_url),
    base::Bind(&PepperFlashBrowserHost::GetLocalDataRestrictions,
               weak_factory_.GetWeakPtr(),
               context->MakeReplyMessageContext()));

  return PP_OK_COMPLETIONPENDING;
}

void PepperFlashBrowserHost::GetLocalDataRestrictions(
    ppapi::host::ReplyMessageContext reply_context,
    int32_t restrictions) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

  SendReply(reply_context,
            PpapiPluginMsg_Flash_GetLocalDataRestrictionsReply(restrictions));
}

} // namespace oxide
