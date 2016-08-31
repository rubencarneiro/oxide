// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_DEVICE_DEVICE_CLIENT_H_
#define _OXIDE_SHARED_BROWSER_DEVICE_DEVICE_CLIENT_H_

#include <memory>

#include "base/macros.h"
#include "device/core/device_client.h"

namespace oxide {

class PowerSaveBlockerService;

class DeviceClient : public device::DeviceClient {
 public:
  DeviceClient();
  ~DeviceClient() override;

 private:
  // device::DeviceClient implementation
  device::UsbService* GetUsbService() override;
  device::HidService* GetHidService() override;
  device::PowerSaveBlockerService* GetPowerSaveBlockerService() override;

  std::unique_ptr<PowerSaveBlockerService> power_save_blocker_service_;

  DISALLOW_COPY_AND_ASSIGN(DeviceClient);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_DEVICE_DEVICE_CLIENT_H_
