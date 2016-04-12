// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_NOTIFICATIONS_NOTIFICATION_DELEGATE_PROXY_H_
#define _OXIDE_SHARED_BROWSER_NOTIFICATIONS_NOTIFICATION_DELEGATE_PROXY_H_

#include <memory>

#include "base/macros.h"
#include "base/memory/ref_counted.h"

namespace content {
class DesktopNotificationDelegate;
}

namespace oxide {

// This class exists to make DesktopNotificationDelegate look like it's
// reference counted, which allows us to attempt to pass it to multiple
// handlers
class NotificationDelegateProxy
    : public base::RefCounted<NotificationDelegateProxy> {
 public:
  NotificationDelegateProxy(
      std::unique_ptr<content::DesktopNotificationDelegate> delegate);

  void NotificationDisplayed();
  void NotificationClosed();
  void NotificationClick();

 private:
  friend class base::RefCounted<NotificationDelegateProxy>;
  ~NotificationDelegateProxy();

  std::unique_ptr<content::DesktopNotificationDelegate> delegate_;

  DISALLOW_COPY_AND_ASSIGN(NotificationDelegateProxy);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_NOTIFICATIONS_NOTIFICATION_DELEGATE_PROXY_H_
