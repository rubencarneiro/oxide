// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (c) 2013 The Chromium Authors. All rights reserved.
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

#include "oxide_pepper_flash_browser_host.h"
#include "oxide_browser_context.h"

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


namespace oxide {

namespace {

// Get the CookieSettings on the UI thread for the given render process ID.
scoped_refptr<oxide::BrowserContext> GetCookieSettings(int render_process_id) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  content::RenderProcessHost* render_process_host =
      content::RenderProcessHost::FromID(render_process_id);

  if (render_process_host && render_process_host->GetBrowserContext()) {
    return BrowserContext::FromContent(render_process_host->GetBrowserContext());
  }

  return NULL;
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

  if (browser_context_.get()) {
    // XXX: do the same rule apply regarding thread and access?
    GetLocalDataRestrictions(context->MakeReplyMessageContext(),
                              document_url,
                              plugin_url,
                              browser_context_);
  } else {
      content::BrowserThread::PostTaskAndReplyWithResult(
        content::BrowserThread::UI,
        FROM_HERE,
        base::Bind(&GetCookieSettings, render_process_id_),
        base::Bind(&PepperFlashBrowserHost::GetLocalDataRestrictions,
                   weak_factory_.GetWeakPtr(),
                   context->MakeReplyMessageContext(),
                   document_url,
                   plugin_url));
  }

  return PP_OK_COMPLETIONPENDING;
}

void PepperFlashBrowserHost::GetLocalDataRestrictions(
    ppapi::host::ReplyMessageContext reply_context,
    const GURL& document_url,
    const GURL& plugin_url,
    scoped_refptr<BrowserContext> browser_context) {

  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

  if (!browser_context_.get()) {
    browser_context_ = browser_context;
  } else {
    DCHECK(browser_context_.get() == browser_context.get());
  }

  PP_FlashLSORestrictions restrictions = PP_FLASHLSORESTRICTIONS_NONE;
  if (browser_context_.get() && document_url.is_valid()) {
    BrowserContextIOData* io_data = browser_context_->io_data();
    if (io_data->GetSessionCookieMode() == content::CookieStoreConfig::EPHEMERAL_SESSION_COOKIES)
      restrictions = PP_FLASHLSORESTRICTIONS_IN_MEMORY;
    if (!io_data->CanAccessCookies(document_url, GURL(), false))
      restrictions = PP_FLASHLSORESTRICTIONS_BLOCK;
  }

  SendReply(reply_context,
            PpapiPluginMsg_Flash_GetLocalDataRestrictionsReply(
                static_cast<int32_t>(restrictions)));
}

} // namespace oxide
