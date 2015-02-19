// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_PORT_GPU_CONFIG_GPU_INFO_COLLECTOR_OXIDE_LINUX_H_
#define _OXIDE_SHARED_PORT_GPU_CONFIG_GPU_INFO_COLLECTOR_OXIDE_LINUX_H_

#include "gpu/config/gpu_info.h"

namespace gpu {

// We can't use the default linux implementation of the GPU info collector
// because it also has to work on Ubuntu phone, where the graphics stack
// is much closer to Android. This is a bit of a hack, but this interface
// lets us implement our own GPU info collector in Oxide, where it has access
// to the AndroidProperties class (I suppose we could have also built that as
// part of base, but that requires an additional Chromium patch)
class GpuInfoCollectorOxideLinux {
 public:
  virtual ~GpuInfoCollectorOxideLinux() {}

  virtual CollectInfoResult CollectGpuID(uint32* vendor_id, uint32* device_id) = 0;

  virtual CollectInfoResult CollectContextGraphicsInfo(GPUInfo* gpu_info) = 0;

  virtual CollectInfoResult CollectBasicGraphicsInfo(GPUInfo* gpu_info) = 0;

  virtual CollectInfoResult CollectDriverInfoGL(GPUInfo* gpu_info) = 0;
};

// Sets the GPU info collector implementation. The caller retains ownership
GPU_EXPORT void SetGpuInfoCollectorOxideLinux(GpuInfoCollectorOxideLinux* collector);

} // namespace gpu

#endif // _OXIDE_SHARED_PORT_GPU_CONFIG_GPU_INFO_COLLECTOR_OXIDE_LINUX_H_
