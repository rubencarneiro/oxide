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

#ifndef _OXIDE_SHARED_BROWSER_PEPPER_TALK_HOST_H_
#define _OXIDE_SHARED_BROWSER_PEPPER_TALK_HOST_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ppapi/c/private/ppb_talk_private.h"
#include "ppapi/host/resource_host.h"

namespace content {
class BrowserPpapiHost;
}

namespace oxide {

class PepperTalkHost FINAL : public ppapi::host::ResourceHost {
 public:
  PepperTalkHost(content::BrowserPpapiHost* host,
                 PP_Instance instance,
                 PP_Resource resource);
  ~PepperTalkHost();

 private:
  int32_t OnResourceMessageReceived(
      const IPC::Message& msg,
      ppapi::host::HostMessageContext* context) FINAL;

  int32_t OnRequestPermission(
      ppapi::host::HostMessageContext* context,
      PP_TalkPermission permission);
  int32_t OnStartRemoting(
      ppapi::host::HostMessageContext* context);
  int32_t OnStopRemoting(
      ppapi::host::HostMessageContext* context);

  DISALLOW_IMPLICIT_CONSTRUCTORS(PepperTalkHost);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_PEPPER_TALK_HOST_H_
