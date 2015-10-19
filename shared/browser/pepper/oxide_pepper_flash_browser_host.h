// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _OXIDE_SHARED_BROWSER_PEPPER_FLASH_BROWSER_HOST_H_
#define _OXIDE_SHARED_BROWSER_PEPPER_FLASH_BROWSER_HOST_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/weak_ptr.h"
#include "ppapi/host/resource_host.h"

class CookieSettings;
class GURL;

namespace base {
class Time;
}

namespace content {
class BrowserPpapiHost;
}

namespace oxide {

class BrowserContext;


class PepperFlashBrowserHost final : public ppapi::host::ResourceHost {
 public:
  PepperFlashBrowserHost(content::BrowserPpapiHost* host,
                         PP_Instance instance,
                         PP_Resource resource);
  ~PepperFlashBrowserHost();

 private:
  int32_t OnResourceMessageReceived(
      const IPC::Message& msg,
      ppapi::host::HostMessageContext* context) override;

  int32_t OnUpdateActivity(ppapi::host::HostMessageContext* host_context);
  int32_t OnGetLocalTimeZoneOffset(
      ppapi::host::HostMessageContext* host_context,
      const base::Time& t);
  int32_t OnGetLocalDataRestrictions(ppapi::host::HostMessageContext* context);

  void GetLocalDataRestrictions(ppapi::host::ReplyMessageContext reply_context,
                                int32_t restrictions);
  content::BrowserPpapiHost* host_;
  int render_process_id_;
  base::WeakPtrFactory<PepperFlashBrowserHost> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(PepperFlashBrowserHost);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_PEPPER_FLASH_BROWSER_HOST_H_
