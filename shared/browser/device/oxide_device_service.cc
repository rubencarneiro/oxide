// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2017 Canonical Ltd.

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

#include "oxide_device_service.h"

#include "base/memory/ptr_util.h"
#include "device/vibration/vibration_manager.mojom.h"

#include "shared/browser/oxide_browser_platform_integration.h"

namespace oxide {

void DeviceService::Create(const service_manager::Identity& remote_identity,
                           device::mojom::VibrationManagerRequest request) {
  BrowserPlatformIntegration::GetInstance()
      ->CreateVibrationManager(std::move(request));
}

DeviceService::DeviceService(
    scoped_refptr<base::SingleThreadTaskRunner> file_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> io_task_runner)
    : device::DeviceService(file_task_runner, io_task_runner) {}

DeviceService::~DeviceService() = default;

std::unique_ptr<service_manager::Service> CreateDeviceService(
    scoped_refptr<base::SingleThreadTaskRunner> file_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> io_task_runner) {
  return base::MakeUnique<DeviceService>(std::move(file_task_runner),
                                         std::move(io_task_runner));
}

} // namespace oxide
