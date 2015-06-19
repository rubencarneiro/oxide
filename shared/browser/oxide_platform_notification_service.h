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

#ifndef _OXIDE_QT_CORE_PLATFORM_NOTIFICATION_H_
#define _OXIDE_QT_CORE_PLATFORM_NOTIFICATION_H_

#include <string>

#include "base/memory/singleton.h"
#include "base/values.h"
#include "content/public/browser/platform_notification_service.h"

namespace oxide {

class PlatformNotificationService: public content::PlatformNotificationService {
public:
  void Initialize(const std::string &name);

  blink::WebNotificationPermission CheckPermissionOnIOThread(
      content::ResourceContext* resource_context,
      const GURL& origin,
      int render_process_id) override;

  blink::WebNotificationPermission CheckPermissionOnUIThread(
      content::BrowserContext* browser_context,
      const GURL& origin,
      int render_process_id) override;

  void DisplayNotification(
      content::BrowserContext* browser_context,
      const GURL& origin,
      const SkBitmap& icon,
      const content::PlatformNotificationData& notification_data,
      scoped_ptr<content::DesktopNotificationDelegate> delegate,
      base::Closure* cancel_callback) override;

  void DisplayPersistentNotification(
      content::BrowserContext* browser_context,
      int64 service_worker_registration_id,
      const GURL& origin,
      const SkBitmap& icon,
      const content::PlatformNotificationData& notification_data) override;

  void ClosePersistentNotification(
      content::BrowserContext* browser_context,
      int64_t persistent_notification_id) override;

  bool GetDisplayedPersistentNotifications(
      content::BrowserContext* browser_context,
      std::set<std::string>* displayed_notifications) override;

  static PlatformNotificationService* GetInstance();
private:
  friend struct DefaultSingletonTraits<PlatformNotificationService>;
  PlatformNotificationService() = default;

  DISALLOW_COPY_AND_ASSIGN(PlatformNotificationService);
};


}

#endif
