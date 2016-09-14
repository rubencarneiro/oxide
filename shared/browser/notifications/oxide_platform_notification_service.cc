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

#include "oxide_platform_notification_service.h"

#include <utility>

#include "base/logging.h"
#include "content/public/browser/desktop_notification_delegate.h"
#include "content/public/common/notification_resources.h"
#include "content/public/common/platform_notification_data.h"

#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/permissions/oxide_temporary_saved_permission_context.h"

#include "oxide_notification_data.h"
#include "oxide_notification_id.h"
#include "oxide_notification_delegate_proxy.h"
#include "oxide_system_notification_dispatcher.h"

namespace oxide {

namespace {

// FIXME(chrisccoulson): Split out of here. This code is duplicated in
//  oxide_permission_manager.cc
blink::mojom::PermissionStatus ToPermissionStatus(
    TemporarySavedPermissionStatus status) {
  switch (status) {
    case TEMPORARY_SAVED_PERMISSION_STATUS_ALLOWED:
      return blink::mojom::PermissionStatus::GRANTED;
    case TEMPORARY_SAVED_PERMISSION_STATUS_DENIED:
      return blink::mojom::PermissionStatus::DENIED;
    case TEMPORARY_SAVED_PERMISSION_STATUS_ASK:
      return blink::mojom::PermissionStatus::ASK;
    default:
      NOTREACHED();
      return blink::mojom::PermissionStatus::DENIED;
  }
}

}

PlatformNotificationService::PlatformNotificationService()
    : system_notification_dispatcher_(SystemNotificationDispatcher::Create()) {}

void PlatformNotificationService::CancelNotificationByID(
    const NotificationID& notification_id) {
  // In the future when we have application handlers for notifications, we'll
  // need more context here so we know what dispatcher handled it
  system_notification_dispatcher_->CloseNotification(notification_id);
}

blink::mojom::PermissionStatus
PlatformNotificationService::CheckPermissionOnUIThread(
    content::BrowserContext* browser_context,
    const GURL& origin,
    int render_process_id) {
  BrowserContext* context = static_cast<BrowserContext*>(browser_context);
  TemporarySavedPermissionContext* permission_context =
      context->GetTemporarySavedPermissionContext();

  return ToPermissionStatus(
      permission_context->GetPermissionStatus(
        TEMPORARY_SAVED_PERMISSION_TYPE_NOTIFICATIONS,
        origin,
        origin));
}

blink::mojom::PermissionStatus
PlatformNotificationService::CheckPermissionOnIOThread(
    content::ResourceContext* resource_context,
    const GURL& origin,
    int render_process_id) {
  TemporarySavedPermissionContext* permission_context =
      BrowserContextIOData::FromResourceContext(resource_context)
        ->GetTemporarySavedPermissionContext();

  return ToPermissionStatus(
      permission_context->GetPermissionStatus(
        TEMPORARY_SAVED_PERMISSION_TYPE_NOTIFICATIONS,
        origin,
        origin));
}

void PlatformNotificationService::DisplayNotification(
    content::BrowserContext* browser_context,
    const std::string& notification_id,
    const GURL& origin,
    const content::PlatformNotificationData& notification_data,
    const content::NotificationResources& notification_resources,
    std::unique_ptr<content::DesktopNotificationDelegate> delegate,
    base::Closure* cancel_callback) {
  NotificationID id(BrowserContext::FromContent(browser_context)->GetID(),
                    origin,
                    notification_data.tag);
  NotificationData data(notification_data.title,
                        notification_data.body,
                        notification_resources.notification_icon);
  scoped_refptr<NotificationDelegateProxy> delegate_proxy =
      new NotificationDelegateProxy(std::move(delegate));

  // In the future we might have the ability to let the application handle
  // this notification, in which case we'll look up a NotificationDispatcher
  // from the BrowserContext here and try that first (although, it would be
  // really nice if we could identify the WebContents it came from first). If
  // it handles it, we should probably call CloseNotification on the system
  // handler

  if (!system_notification_dispatcher_->DisplayNotification(id,
                                                            data,
                                                            delegate_proxy)) {
    return;
  }

  if (cancel_callback) {
    *cancel_callback =
        base::Bind(&PlatformNotificationService::CancelNotificationByID,
                   base::Unretained(this), id);
  }
}

void PlatformNotificationService::DisplayPersistentNotification(
    content::BrowserContext* browser_context, 
    const std::string& notification_id,
    const GURL& service_worker_origin,
    const GURL& origin,
    const content::PlatformNotificationData& notification_data,
    const content::NotificationResources& notification_resources) {
  NOTIMPLEMENTED();
}

void PlatformNotificationService::ClosePersistentNotification(
    content::BrowserContext* browser_context,
    const std::string& notification_id) {
  NOTIMPLEMENTED();
}

bool PlatformNotificationService::GetDisplayedPersistentNotifications(
    content::BrowserContext* browser_context,
    std::set<std::string>* displayed_notifications) {
  return false;
}

PlatformNotificationService* PlatformNotificationService::GetInstance() {
  return base::Singleton<PlatformNotificationService>::get();
}

}
