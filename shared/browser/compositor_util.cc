// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#include "base/values.h"
#include "content/browser/gpu/compositor_util.h"
#include "content/browser/gpu/gpu_data_manager_impl.h"

namespace content {

bool IsThreadedCompositingEnabled() {
  return false;
}

bool IsForceCompositingModeEnabled() {
  return false;
}

bool IsDelegatedRendererEnabled() {
  return false;
}

bool IsDeadlineSchedulingEnabled() {
  return true;
}

base::Value* GetFeatureStatus() {
  // Currently only used by chrome://gpu
  return NULL;
}

base::Value* GetProblems() {
  // Currently only used by chrome://gpu
  return NULL;
}

base::Value* GetDriverBugWorkarounds() {
  base::ListValue* workaround_list = new base::ListValue();
  GpuDataManagerImpl::GetInstance()->GetDriverBugWorkarounds(workaround_list);
  return workaround_list;
}

} // namespace content
