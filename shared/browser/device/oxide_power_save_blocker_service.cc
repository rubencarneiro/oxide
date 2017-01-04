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

#include "oxide_power_save_blocker_service.h"

#include <memory>

#include "power_save_blocker.h"

namespace oxide {

int PowerSaveBlockerService::CreatePowerSaveBlocker(
    device::PowerSaveBlocker::PowerSaveBlockerType type,
    device::PowerSaveBlocker::Reason reason,
    const std::string& description) {
  std::unique_ptr<PowerSaveBlocker> blocker =
      PowerSaveBlocker::Create(type, reason, description);
  if (!blocker) {
    return -1;
  }

  return blockers_.Add(std::move(blocker));
}

void PowerSaveBlockerService::CancelPowerSaveBlocker(int id) {
  blockers_.Remove(id);
}

PowerSaveBlockerService::PowerSaveBlockerService() = default;

PowerSaveBlockerService::~PowerSaveBlockerService() = default;

} // namespace oxide
