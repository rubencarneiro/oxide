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

#ifndef _OXIDE_SHARED_BROWSER_DEVICE_DEVICE_SERVICE_H_
#define _OXIDE_SHARED_BROWSER_DEVICE_DEVICE_SERVICE_H_

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "services/device/device_service.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace oxide {

// This is a bit of a hack to allow us to override device::VibrationManager
class DeviceService : public device::DeviceService {
 public:
  DeviceService(scoped_refptr<base::SingleThreadTaskRunner> file_task_runner,
                scoped_refptr<base::SingleThreadTaskRunner> io_task_runner);
  ~DeviceService() override;

 private:
  // service_manager::InterfaceFactory<mojom::VibrationManager> implementation
  void Create(const service_manager::Identity& remote_identity,
              device::mojom::VibrationManagerRequest request) override;

  DISALLOW_COPY_AND_ASSIGN(DeviceService);
};

std::unique_ptr<service_manager::Service> CreateDeviceService(
    scoped_refptr<base::SingleThreadTaskRunner> file_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> io_task_runner);

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_DEVICE_DEVICE_SERVICE_H_
