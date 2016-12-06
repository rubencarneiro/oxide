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

#ifndef _OXIDE_SHARED_BROWSER_DEVICE_POWER_SAVE_BLOCKER_SERVICE_H_
#define _OXIDE_SHARED_BROWSER_DEVICE_POWER_SAVE_BLOCKER_SERVICE_H_

#include <memory>

#include "base/id_map.h"
#include "base/macros.h"
#include "device/power_save_blocker/power_save_blocker_service.h"

namespace oxide {

class PowerSaveBlocker;

class PowerSaveBlockerService : public device::PowerSaveBlockerService {
 public:
  PowerSaveBlockerService();
  ~PowerSaveBlockerService() override;

 private:
  // device::PowerSaveBlockerService implementation
  int CreatePowerSaveBlocker(
      device::PowerSaveBlocker::PowerSaveBlockerType type,
      device::PowerSaveBlocker::Reason reason,
      const std::string& description) override;
  void CancelPowerSaveBlocker(int id) override;

  IDMap<std::unique_ptr<PowerSaveBlocker>> blockers_;

  DISALLOW_COPY_AND_ASSIGN(PowerSaveBlockerService);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_DEVICE_POWER_SAVE_BLOCKER_SERVICE_H_
