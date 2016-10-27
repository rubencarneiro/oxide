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

#ifndef _OXIDE_SHARED_BROWSER_NAVIGATION_CONTROLLER_OBSERVER_H_
#define _OXIDE_SHARED_BROWSER_NAVIGATION_CONTROLLER_OBSERVER_H_

#include "base/macros.h"
#include "content/public/browser/invalidate_type.h"

#include "shared/common/oxide_shared_export.h"

namespace content {
class NavigationController;
}

namespace oxide {

class OXIDE_SHARED_EXPORT NavigationControllerObserver {
 public:
  NavigationControllerObserver();
  NavigationControllerObserver(content::NavigationController* controller);

  virtual ~NavigationControllerObserver();

  static void NotifyNavigationStateChanged(
      const content::NavigationController& controller,
      content::InvalidateTypes change_flags);

  virtual void NavigationHistoryChanged() = 0;

  // public for DEFINE_WEB_CONTENTS_USER_DATA_KEY macro
  class Delegate;

 protected:
  void Observe(content::NavigationController* controller);

 private:
  void OnDelegateDestruction();

  Delegate* delegate_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(NavigationControllerObserver);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_NAVIGATION_CONTROLLER_OBSERVER_H_
