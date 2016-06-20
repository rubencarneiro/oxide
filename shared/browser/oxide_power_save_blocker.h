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

#ifndef _OXIDE_SHARED_BROWSER_POWER_SAVE_BLOCKER_H_
#define _OXIDE_SHARED_BROWSER_POWER_SAVE_BLOCKER_H_

#include <string>

#include "base/memory/ref_counted.h"
#include "device/power_save_blocker/power_save_blocker.h"

namespace base {
class SequencedTaskRunner;
class SingleThreadTaskRunner;
}

namespace device {
class PowerSaveBlockerOxideDelegate;
}

namespace oxide {

device::PowerSaveBlockerOxideDelegate* CreatePowerSaveBlocker(
    device::PowerSaveBlocker::PowerSaveBlockerType type,
    device::PowerSaveBlocker::Reason reason,
    const std::string& description,
    scoped_refptr<base::SequencedTaskRunner> ui_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> blocking_task_runner);

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_POWER_SAVE_BLOCKER_H_
