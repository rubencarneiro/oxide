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

#ifndef _OXIDE_SHARED_BROWSER_NOTIFICATIONS_PLATFORM_NOTIFICATION_SERVICE_H_
#define _OXIDE_SHARED_BROWSER_NOTIFICATIONS_PLATFORM_NOTIFICATION_SERVICE_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "content/public/browser/platform_notification_service.h"

namespace base {
template <typename T> struct DefaultSingletonTraits;
}

namespace oxide {

struct NotificationID;
class SystemNotificationDispatcher;

class PlatformNotificationService: public content::PlatformNotificationService {
 public:
  static PlatformNotificationService* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<PlatformNotificationService>;

  PlatformNotificationService();

  void CancelNotificationByID(const NotificationID& notification_id);

  // content::PlatformNotificationService implementation
  blink::mojom::PermissionStatus CheckPermissionOnUIThread(
      content::BrowserContext* browser_context,
      const GURL& origin,
      int render_process_id) override;
  blink::mojom::PermissionStatus CheckPermissionOnIOThread(
      content::ResourceContext* resource_context,
      const GURL& origin,
      int render_process_id) override;
  void DisplayNotification(
      content::BrowserContext* browser_context,
      const std::string& notification_id,
      const GURL& origin,
      const content::PlatformNotificationData& notification_data,
      const content::NotificationResources& notification_resources,
      std::unique_ptr<content::DesktopNotificationDelegate> delegate,
      base::Closure* cancel_callback) override;
  void DisplayPersistentNotification(
      content::BrowserContext* browser_context,
      const std::string& notification_id,
      const GURL& service_worker_origin,
      const GURL& origin,
      const content::PlatformNotificationData& notification_data,
      const content::NotificationResources& notification_resources) override;
  void ClosePersistentNotification(
      content::BrowserContext* browser_context,
      const std::string& notification_id) override;
  void GetDisplayedNotifications(
      content::BrowserContext* browser_context,
      const DisplayedNotificationsCallback& callback) override;

  std::unique_ptr<SystemNotificationDispatcher> system_notification_dispatcher_;

  DISALLOW_COPY_AND_ASSIGN(PlatformNotificationService);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_NOTIFICATIONS_PLATFORM_NOTIFICATION_SERVICE_H_
