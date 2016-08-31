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

#ifndef _OXIDE_SHARED_BROWSER_DEVICE_POWER_SAVE_BLOCKER_H_
#define _OXIDE_SHARED_BROWSER_DEVICE_POWER_SAVE_BLOCKER_H_

#include <memory>
#include <string>

#include "device/power_save_blocker/power_save_blocker.h"

namespace oxide {

class PowerSaveBlocker {
 public:
  virtual ~PowerSaveBlocker();

  static std::unique_ptr<PowerSaveBlocker> Create(
      device::PowerSaveBlocker::PowerSaveBlockerType type,
      device::PowerSaveBlocker::Reason,
      const std::string& description);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_DEVICE_POWER_SAVE_BLOCKER_H_
