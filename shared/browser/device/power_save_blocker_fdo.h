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

#ifndef _OXIDE_SHARED_BROWSER_DEVICE_POWER_SAVE_BLOCKER_FDO_H_
#define _OXIDE_SHARED_BROWSER_DEVICE_POWER_SAVE_BLOCKER_FDO_H_

#include "base/macros.h"
#include "base/memory/ref_counted.h"

#include "shared/browser/device/power_save_blocker.h"

namespace oxide {

class PowerSaveBlockerFDO : public PowerSaveBlocker {
 public:
  PowerSaveBlockerFDO(
      device::PowerSaveBlocker::PowerSaveBlockerType type,
      const std::string& description);
  ~PowerSaveBlockerFDO() override;

 private:
  class Core;
  scoped_refptr<Core> core_;

  DISALLOW_COPY_AND_ASSIGN(PowerSaveBlockerFDO);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_DEVICE_POWER_SAVE_BLOCKER_FDO_H_
