// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#include "gpu_client_shim_oxide.h"

#include "content/common/gpu/client/command_buffer_proxy_impl.h"
#include "content/common/gpu/client/context_provider_command_buffer.h"

namespace content {
namespace oxide_gpu_shim {

int32_t GetContextProviderRouteID(
    content::ContextProviderCommandBuffer* provider) {
  return provider->GetCommandBufferProxy()->GetRouteID();
}

} // namespace oxide_gpu_shim
} // namespace content
