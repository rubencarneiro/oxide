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

#ifndef _OXIDE_SHARED_BROWSER_SCREEN_H_
#define _OXIDE_SHARED_BROWSER_SCREEN_H_

#include <vector>

#include "base/observer_list.h"
#include "base/threading/thread_checker.h"
#include "ui/display/display.h"
#include "ui/gfx/geometry/point.h"

#include "shared/common/oxide_shared_export.h"

namespace oxide {

class ScreenObserver;
enum class DisplayFormFactor;
enum class ShellMode;

// A central place to track screen state - we have to provide access to the
// default display anyway, so this avoids having to duplicate work for
// WebContentsView, which also needs screen state
class OXIDE_SHARED_EXPORT Screen {
 public:
  static Screen* GetInstance();

  virtual ~Screen();

  virtual display::Display GetPrimaryDisplay() = 0;

  virtual std::vector<display::Display> GetAllDisplays() = 0;

  virtual gfx::Point GetCursorScreenPoint() = 0;

  virtual DisplayFormFactor GetDisplayFormFactor(
      const display::Display& display);

  virtual ShellMode GetShellMode();

 protected:
  Screen();

  void NotifyPrimaryDisplayChanged();
  void NotifyDisplayAdded(const display::Display& display);
  void NotifyDisplayRemoved(const display::Display& display);
  void NotifyDisplayPropertiesChanged(const display::Display& display);
  void NotifyShellModeChanged();

 private:
  friend class ScreenObserver;

  void AddObserver(ScreenObserver* observer);
  void RemoveObserver(ScreenObserver* observer);

  base::ThreadChecker thread_checker_;

  base::ObserverList<ScreenObserver> observers_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_SCREEN_H_
