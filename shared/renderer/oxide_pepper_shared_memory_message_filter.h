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

#ifndef CHROME_RENDERER_PEPPER_PEPPER_SHARED_MEMORY_MESSAGE_FILTER_H_
#define CHROME_RENDERER_PEPPER_PEPPER_SHARED_MEMORY_MESSAGE_FILTER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ppapi/c/pp_instance.h"
#include "ppapi/host/instance_message_filter.h"

namespace content {
class RendererPpapiHost;
}

namespace ppapi {
namespace proxy {
class SerializedHandle;
}
}

// Implements the backend for shared memory messages from a plugin process.
class PepperSharedMemoryMessageFilter
    : public ppapi::host::InstanceMessageFilter {
 public:
  explicit PepperSharedMemoryMessageFilter(content::RendererPpapiHost* host);
  virtual ~PepperSharedMemoryMessageFilter();

  // InstanceMessageFilter:
  virtual bool OnInstanceMessageReceived(const IPC::Message& msg) OVERRIDE;

  bool Send(IPC::Message* msg);

 private:
  // Message handlers.
  void OnHostMsgCreateSharedMemory(
      PP_Instance instance,
      uint32_t size,
      int* host_shm_handle_id,
      ppapi::proxy::SerializedHandle* plugin_shm_handle);

  content::RendererPpapiHost* host_;

  DISALLOW_COPY_AND_ASSIGN(PepperSharedMemoryMessageFilter);
};

#endif  // CHROME_RENDERER_PEPPER_PEPPER_SHARED_MEMORY_MESSAGE_FILTER_H_
