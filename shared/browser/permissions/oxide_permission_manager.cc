// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#include "oxide_permission_manager.h"

#include "base/bind.h"
#include "content/public/browser/geolocation_provider.h"
#include "content/public/browser/permission_type.h"
#include "content/public/common/permission_status.mojom.h"

#include "shared/browser/notifications/oxide_platform_notification_service.h"
#include "shared/browser/oxide_browser_context.h"

#include "oxide_permission_request_dispatcher.h"
#include "oxide_permission_request_response.h"
#include "oxide_temporary_saved_permission_context.h"

namespace oxide {

namespace {

content::PermissionStatus ToPermissionStatus(
    PermissionRequestResponse response) {
  return response == PERMISSION_REQUEST_RESPONSE_ALLOW ?
      content::PERMISSION_STATUS_GRANTED :
      content::PERMISSION_STATUS_DENIED;
}

content::PermissionStatus ToPermissionStatus(
    TemporarySavedPermissionStatus status) {
  switch (status) {
    case TEMPORARY_SAVED_PERMISSION_STATUS_ALLOWED:
      return content::PERMISSION_STATUS_GRANTED;
    case TEMPORARY_SAVED_PERMISSION_STATUS_DENIED:
      return content::PERMISSION_STATUS_DENIED;
    case TEMPORARY_SAVED_PERMISSION_STATUS_ASK:
      return content::PERMISSION_STATUS_ASK;
    default:
      NOTREACHED();
      return content::PERMISSION_STATUS_DENIED;
  }
}

bool IsPermissionTypeSupported(content::PermissionType permission) {
  switch (permission) {
    case content::PermissionType::GEOLOCATION:
    case content::PermissionType::NOTIFICATIONS:
      return true;
    default:
      return false;
  }
}

TemporarySavedPermissionType ToTemporarySavedPermissionType(
    content::PermissionType permission) {
  switch (permission) {
    case content::PermissionType::GEOLOCATION:
      return TEMPORARY_SAVED_PERMISSION_TYPE_GEOLOCATION;
    case content::PermissionType::NOTIFICATIONS:
      return TEMPORARY_SAVED_PERMISSION_TYPE_NOTIFICATIONS;
    default:
      NOTREACHED();
      // XXX(chrisccoulson): Perhaps we need __builtin_unreachable here?
      return static_cast<TemporarySavedPermissionType>(-1);
  }
}

TemporarySavedPermissionStatus ToTemporarySavedPermissionStatus(
    PermissionRequestResponse response) {
  DCHECK_NE(response, PERMISSION_REQUEST_RESPONSE_CANCEL);
  return response == PERMISSION_REQUEST_RESPONSE_ALLOW ?
      TEMPORARY_SAVED_PERMISSION_STATUS_ALLOWED :
      TEMPORARY_SAVED_PERMISSION_STATUS_DENIED;
}

void RespondToGeolocationPermissionRequest(
    const PermissionRequestCallback& callback,
    PermissionRequestResponse response) {
  if (response == PERMISSION_REQUEST_RESPONSE_ALLOW) {
    content::GeolocationProvider::GetInstance()
        ->UserDidOptIntoLocationServices();
  }
  callback.Run(response);
}

void RespondToPermissionRequest(
    const base::Callback<void(content::PermissionStatus)>& callback,
    content::PermissionType permission,
    BrowserContext* context,
    const GURL& requesting_origin,
    const GURL& embedding_origin,
    PermissionRequestResponse response) {
  callback.Run(ToPermissionStatus(response));

  if (response == PERMISSION_REQUEST_RESPONSE_CANCEL) {
    return;
  }

  context->GetTemporarySavedPermissionContext()->SetPermissionStatus(
      ToTemporarySavedPermissionType(permission),
      requesting_origin,
      embedding_origin,
      ToTemporarySavedPermissionStatus(response));
}

const PermissionRequestCallback WrapCallback(
    const base::Callback<void(content::PermissionStatus)>& callback,
    content::PermissionType permission,
    BrowserContext* context,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
  const PermissionRequestCallback wrapped_callback =
      base::Bind(&RespondToPermissionRequest,
                 callback,
                 permission,
                 // The owner of PermissionRequestDispatcher keeps |context|
                 // alive. The request will be cancelled by
                 // PermissionRequestDispatcher when it is destroyed, so it's
                 // ok to not have a strong reference here
                 base::Unretained(context),
                 requesting_origin,
                 embedding_origin);
  switch (permission) {
    case content::PermissionType::GEOLOCATION:
      return base::Bind(&RespondToGeolocationPermissionRequest,
                        wrapped_callback);
    default:
      return wrapped_callback;
  }
}

}

struct PermissionManager::Subscription {
  base::Callback<void(content::PermissionStatus)> callback;
};

void PermissionManager::RequestPermission(
    content::PermissionType permission,
    content::RenderFrameHost* render_frame_host,
    int request_id,
    const GURL& requesting_origin,
    bool user_gesture,
    const base::Callback<void(content::PermissionStatus)>& callback) {
  if (!IsPermissionTypeSupported(permission)) {
    callback.Run(content::PERMISSION_STATUS_DENIED);
    return;
  }

  content::WebContents* web_contents =
      content::WebContents::FromRenderFrameHost(render_frame_host);
  if (!web_contents) {
    callback.Run(content::PERMISSION_STATUS_DENIED);
    return;
  }

  GURL embedding_origin = web_contents->GetLastCommittedURL().GetOrigin();
  if (permission == content::PermissionType::NOTIFICATIONS) {
    // Use the requesting origin as the embedding origin for notifications,
    // as we don't get an embedding origin in
    // PlatformNotificationService::CheckPermissionOn{UI,IO}Thread
    // XXX(chrisccoulson): I don't really like having PermissionType specific
    //  code here in PermissionManager. PermissionManager should probably
    //  delegate this to PermissionType specific classes (same for geolocation
    //  response handling above)
    embedding_origin = requesting_origin;
  }

  TemporarySavedPermissionStatus status =
      context_->GetTemporarySavedPermissionContext()->GetPermissionStatus(
        ToTemporarySavedPermissionType(permission),
        requesting_origin,
        embedding_origin);
  if (status != TEMPORARY_SAVED_PERMISSION_STATUS_ASK) {
    callback.Run(ToPermissionStatus(status));
    return;
  }

  PermissionRequestDispatcher* dispatcher =
      PermissionRequestDispatcher::FromWebContents(web_contents);
  if (!dispatcher) {
    callback.Run(content::PERMISSION_STATUS_DENIED);
    return;
  }

  dispatcher->RequestPermission(
      permission,
      render_frame_host,
      request_id,
      requesting_origin,
      WrapCallback(callback, permission, context_, requesting_origin,
                   embedding_origin));
}

void PermissionManager::CancelPermissionRequest(
    content::PermissionType permission,
    content::RenderFrameHost* render_frame_host,
    int request_id,
    const GURL& requesting_origin) {
  content::WebContents* web_contents =
      content::WebContents::FromRenderFrameHost(render_frame_host);
  if (!web_contents) {
    return;
  }

  PermissionRequestDispatcher* dispatcher =
      PermissionRequestDispatcher::FromWebContents(web_contents);
  if (!dispatcher) {
    return;
  }

  dispatcher->CancelPermissionRequest(permission,
                                      render_frame_host,
                                      request_id,
                                      requesting_origin);
}

content::PermissionStatus PermissionManager::GetPermissionStatus(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
  if (!IsPermissionTypeSupported(permission)) {
    return content::PERMISSION_STATUS_DENIED;
  }

  return ToPermissionStatus(
      context_->GetTemporarySavedPermissionContext()->GetPermissionStatus(
        ToTemporarySavedPermissionType(permission),
        requesting_origin,
        embedding_origin));
}

void PermissionManager::ResetPermission(content::PermissionType permission,
                                        const GURL& requesting_origin,
                                        const GURL& embedding_origin) {
  NOTIMPLEMENTED();
}

void PermissionManager::RegisterPermissionUsage(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {}

int PermissionManager::SubscribePermissionStatusChange(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin,
    const base::Callback<void(content::PermissionStatus)>& callback) {
  // This is currently unused, but the callback owns a pointer that the calling
  // code expects us to keep alive
  Subscription* subscription = new Subscription();
  subscription->callback = callback;

  return subscriptions_.Add(subscription);
}

void PermissionManager::UnsubscribePermissionStatusChange(int subscription_id) {
  subscriptions_.Remove(subscription_id);
}

PermissionManager::PermissionManager(BrowserContext* context)
    : context_(context) {}

PermissionManager::~PermissionManager() {}

} // namespace oxide
