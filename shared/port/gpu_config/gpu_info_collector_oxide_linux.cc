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

#include "gpu_info_collector_oxide_linux.h"

#include "base/logging.h"
#include "gpu/config/gpu_info_collector.h"

namespace gpu {

namespace {
GpuInfoCollectorOxideLinux* g_collector;
}

CollectInfoResult CollectGpuID(uint32* vendor_id, uint32* device_id) {
  DCHECK(vendor_id && device_id);
  return g_collector->CollectGpuID(vendor_id, device_id);
}

CollectInfoResult CollectContextGraphicsInfo(GPUInfo* gpu_info) {
  DCHECK(gpu_info);
  return g_collector->CollectContextGraphicsInfo(gpu_info);
}

CollectInfoResult CollectBasicGraphicsInfo(GPUInfo* gpu_info) {
  DCHECK(gpu_info);
  return g_collector->CollectBasicGraphicsInfo(gpu_info);
}

CollectInfoResult CollectDriverInfoGL(GPUInfo* gpu_info) {
  DCHECK(gpu_info);
  return g_collector->CollectDriverInfoGL(gpu_info);
}

void MergeGPUInfo(GPUInfo* basic_gpu_info,
                  const GPUInfo& context_gpu_info) {
  MergeGPUInfoGL(basic_gpu_info, context_gpu_info);
}

void SetGpuInfoCollectorOxideLinux(GpuInfoCollectorOxideLinux* collector) {
  g_collector = collector;
}

} // namespace gpu
