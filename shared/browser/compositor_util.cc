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

#include "content/browser/gpu/compositor_util.h"

#include "base/command_line.h"
#include "content/browser/gpu/gpu_data_manager_impl.h"
#include "content/public/common/content_switches.h"
#include "gpu/config/gpu_feature_type.h"

#include "shared/common/oxide_constants.h"

namespace content {

namespace {

bool CanDoAcceleratedCompositing() {
  const GpuDataManagerImpl* manager = GpuDataManagerImpl::GetInstance();

  // Don't use force compositing mode if gpu access has been blocked or
  // accelerated compositing is blacklisted.
  if (!manager->GpuAccessAllowed(NULL) ||
      manager->IsFeatureBlacklisted(
          gpu::GPU_FEATURE_TYPE_ACCELERATED_COMPOSITING)) {
    return false;
  }

  // Check for SwiftShader.
  if (manager->ShouldUseSwiftShader()) {
    return false;
  }

  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();
  if (command_line.HasSwitch(switches::kDisableAcceleratedCompositing)) {
    return false;
  }

  return true;
}

bool IsForceCompositingModeBlacklisted() {
  return GpuDataManagerImpl::GetInstance()->IsFeatureBlacklisted(
      gpu::GPU_FEATURE_TYPE_FORCE_COMPOSITING_MODE);
}

}

bool IsThreadedCompositingEnabled() {
  const base::CommandLine command_line =
      *base::CommandLine::ForCurrentProcess();

  if (command_line.HasSwitch(switches::kDisableForceCompositingMode) ||
      command_line.HasSwitch(switches::kDisableThreadedCompositing) ||
      !CanDoAcceleratedCompositing() ||
      IsForceCompositingModeBlacklisted()) {
    return false;
  }

  return true;
}

bool IsForceCompositingModeEnabled() {
  if (IsThreadedCompositingEnabled()) {
    return true;
  }

  const base::CommandLine command_line =
      *base::CommandLine::ForCurrentProcess();

  if (command_line.HasSwitch(switches::kDisableForceCompositingMode)) {
    return false;
  }

  if (command_line.HasSwitch(switches::kForceCompositingMode)) {
    return true;
  }

  if (!CanDoAcceleratedCompositing() || IsForceCompositingModeBlacklisted()) {
    return false;
  }

  return true;
}

bool IsDelegatedRendererEnabled() {
  return false;
}

bool IsImplSidePaintingEnabled() {
  return false;
}

bool IsGpuRasterizationEnabled() {
  if (!IsImplSidePaintingEnabled()) {
    return false;
  }

  return false;
}

bool IsForceGpuRasterizationEnabled() {
  if (!IsImplSidePaintingEnabled()) {
    return false;
  }

  return false;
}

base::Value* GetFeatureStatus() {
  return NULL;
}

base::Value* GetProblems() {
  return NULL;
}

base::Value* GetDriverBugWorkarounds() {
  base::ListValue* workaround_list = new base::ListValue();
  GpuDataManagerImpl::GetInstance()->GetDriverBugWorkarounds(workaround_list);
  return workaround_list;
}

} // namespace content
