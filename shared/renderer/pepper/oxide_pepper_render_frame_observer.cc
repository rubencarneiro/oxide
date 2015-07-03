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

#include "content/public/renderer/renderer_ppapi_host.h"
#include "chrome/renderer/pepper/pepper_shared_memory_message_filter.h"
#include "ppapi/host/ppapi_host.h"

#include "oxide_pepper_render_frame_observer.h"
#include "oxide_pepper_renderer_host_factory.h"

namespace oxide {

PepperRenderFrameObserver::PepperRenderFrameObserver(content::RenderFrame* render_frame)
    : RenderFrameObserver(render_frame) {}

PepperRenderFrameObserver::~PepperRenderFrameObserver() {}

void PepperRenderFrameObserver::DidCreatePepperPlugin(content::RendererPpapiHost* host) {

  host->GetPpapiHost()->AddHostFactoryFilter(
      scoped_ptr<ppapi::host::HostFactory>(
          new PepperRendererHostFactory(host)));

  host->GetPpapiHost()->AddInstanceMessageFilter(
      scoped_ptr<ppapi::host::InstanceMessageFilter>(
          new ::PepperSharedMemoryMessageFilter(host)));

}

} // namespace oxide
