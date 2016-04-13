// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _OXIDE_SHARED_PEPPER_RENDERER_HOST_FACTORY_H_
#define _OXIDE_SHARED_PEPPER_RENDERER_HOST_FACTORY_H_

#include "base/macros.h"
#include "ppapi/host/host_factory.h"
#include "content/public/renderer/render_frame_observer.h"

namespace ppapi {
namespace host {
class PpapiHost;
class ResourceHost;
}
}

namespace oxide {

class PepperRendererHostFactory : public ppapi::host::HostFactory {
 public:
  explicit PepperRendererHostFactory(content::RendererPpapiHost* host);
  ~PepperRendererHostFactory();

  // HostFactory.
  std::unique_ptr<ppapi::host::ResourceHost> CreateResourceHost(
      ppapi::host::PpapiHost* host,
      PP_Resource resource,
      PP_Instance instance,
      const IPC::Message& message) override;

 private:
  // Not owned by this object.
  content::RendererPpapiHost* host_;

  DISALLOW_COPY_AND_ASSIGN(PepperRendererHostFactory);
};

} // namespace oxide

#endif // _OXIDE_SHARED_PEPPER_RENDERER_HOST_FACTORY_H_
