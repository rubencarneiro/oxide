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
                                const GURL& document_url,
                                const GURL& plugin_url,
                                scoped_refptr<BrowserContext> browser_context);

  content::BrowserPpapiHost* host_;
  int render_process_id_;
  scoped_refptr<BrowserContext> browser_context_;
  base::WeakPtrFactory<PepperFlashBrowserHost> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(PepperFlashBrowserHost);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_PEPPER_FLASH_BROWSER_HOST_H_

