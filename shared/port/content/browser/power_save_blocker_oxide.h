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

#ifndef _OXIDE_SHARED_PORT_CONTENT_BROWSER_POWER_SAVE_BLOCKER_H_
#define _OXIDE_SHARED_PORT_CONTENT_BROWSER_POWER_SAVE_BLOCKER_H_

#include "base/memory/ref_counted.h"
#include "content/common/content_export.h"
#include "content/public/browser/power_save_blocker.h"

namespace content {

class PowerSaveBlockerOxideDelegate
    : public base::RefCountedThreadSafe<PowerSaveBlockerOxideDelegate> {
 public:
  virtual void Init() = 0;
  virtual void CleanUp() = 0;

 protected:
  friend class base::RefCountedThreadSafe<PowerSaveBlockerOxideDelegate>;
  virtual ~PowerSaveBlockerOxideDelegate() {}
};

typedef PowerSaveBlockerOxideDelegate*
(PowerSaveBlockerOxideDelegateFactory)(PowerSaveBlocker::PowerSaveBlockerType,
                                       const std::string&);

CONTENT_EXPORT void SetPowerSaveBlockerOxideDelegateFactory(
    PowerSaveBlockerOxideDelegateFactory* factory);

} // namespace content

#endif // _OXIDE_SHARED_PORT_CONTENT_BROWSER_POWER_SAVE_BLOCKER_H_
