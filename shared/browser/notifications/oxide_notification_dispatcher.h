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

#ifndef _OXIDE_SHARED_BROWSER_NOTIFICATIONS_NOTIFICATION_DISPATCHER_H_
#define _OXIDE_SHARED_BROWSER_NOTIFICATIONS_NOTIFICATION_DISPATCHER_H_

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"

namespace oxide {

struct NotificationData;
struct NotificationID;
class NotificationDelegateProxy;

// We delegate notifications to an implementation of NotificationDispatcher.
// This interface is provided because it will allow us to provide additional
// implementations in the future which could:
// - Gives the embedding application a chance to intercept and notifications
// - Integrate with other system notification APIs, depending on the desktop
//   environment (eg, we just use libnotify on Linux at the moment but perhaps
//   GNOME or Unity may provide better APIs in the future)
class NotificationDispatcher {
 public:
  virtual ~NotificationDispatcher() {}

  virtual bool DisplayNotification(
      const NotificationID& notification_id,
      const NotificationData& notification_data,
      scoped_refptr<NotificationDelegateProxy> delegate) = 0;

  virtual void CloseNotification(const NotificationID& notification_id) = 0;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_NOTIFICATIONS_NOTIFICATION_DISPATCHER_H_
