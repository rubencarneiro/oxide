// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
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

#ifndef _OXIDE_SHARED_PEPPER_RENDERER_HOST_FACTORY_H_
#define _OXIDE_SHARED_PEPPER_RENDERER_HOST_FACTORY_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ppapi/host/host_factory.h"
#include "content/public/renderer/render_frame_observer.h"


namespace content {
class RenderFrame;
}

namespace oxide {

class PepperRendererHostFactory : public ppapi::host::HostFactory {
 public:
  explicit PepperRendererHostFactory(content::RendererPpapiHost* host);
  ~PepperRendererHostFactory();

  // HostFactory.
  scoped_ptr<ppapi::host::ResourceHost> CreateResourceHost(
      ppapi::host::PpapiHost* host,
      const ppapi::proxy::ResourceMessageCallParams& params,
      PP_Instance instance,
      const IPC::Message& message) OVERRIDE;

 private:
  // Not owned by this object.
  content::RendererPpapiHost* host_;

  DISALLOW_COPY_AND_ASSIGN(PepperRendererHostFactory);
};

} // namespace oxide

#endif // _OXIDE_SHARED_PEPPER_RENDERER_HOST_FACTORY_H_
