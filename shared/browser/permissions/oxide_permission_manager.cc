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
#include "shared/browser/oxide_render_object_id.h"

#include "oxide_permission_request_dispatcher.h"
#include "oxide_temporary_saved_permission_context.h"

namespace oxide {

namespace {

content::PermissionStatus ToPermissionStatus(
    PermissionRequestResponse response) {
  return response == PERMISSION_REQUEST_RESPONSE_ALLOW ?
      content::PermissionStatus::GRANTED :
      content::PermissionStatus::DENIED;
}

content::PermissionStatus ToPermissionStatus(
    TemporarySavedPermissionStatus status) {
  switch (status) {
    case TEMPORARY_SAVED_PERMISSION_STATUS_ALLOWED:
      return content::PermissionStatus::GRANTED;
    case TEMPORARY_SAVED_PERMISSION_STATUS_DENIED:
      return content::PermissionStatus::DENIED;
    case TEMPORARY_SAVED_PERMISSION_STATUS_ASK:
      return content::PermissionStatus::ASK;
    default:
      NOTREACHED();
      return content::PermissionStatus::DENIED;
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

}

struct PermissionManager::Subscription {
  base::Callback<void(content::PermissionStatus)> callback;
};

struct PermissionManager::RequestData {
  RequestData(content::RenderFrameHost* render_frame_host)
      : request_id(-1),
        render_frame_host_id(render_frame_host) {}

  int request_id;
  RenderFrameHostID render_frame_host_id;
};

void PermissionManager::RespondToPermissionRequest(
    int request_id,
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin,
    const base::Callback<void(content::PermissionStatus)>& callback,
    PermissionRequestResponse response) {
  if (permission == content::PermissionType::GEOLOCATION &&
      response == PERMISSION_REQUEST_RESPONSE_ALLOW) {
    content::GeolocationProvider::GetInstance()
        ->UserDidOptIntoLocationServices();
  }

  requests_.Remove(request_id);
  callback.Run(ToPermissionStatus(response));

  if (response == PERMISSION_REQUEST_RESPONSE_CANCEL) {
    return;
  }

  context_->GetTemporarySavedPermissionContext()->SetPermissionStatus(
      ToTemporarySavedPermissionType(permission),
      requesting_origin,
      embedding_origin,
      ToTemporarySavedPermissionStatus(response));
}

int PermissionManager::RequestPermission(
    content::PermissionType permission,
    content::RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    const base::Callback<void(content::PermissionStatus)>& callback) {
  if (!IsPermissionTypeSupported(permission)) {
    callback.Run(content::PermissionStatus::DENIED);
    return kNoPendingOperation;
  }

  content::WebContents* web_contents =
      content::WebContents::FromRenderFrameHost(render_frame_host);
  if (!web_contents) {
    callback.Run(content::PermissionStatus::DENIED);
    return kNoPendingOperation;
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
    return kNoPendingOperation;
  }

  PermissionRequestDispatcher* dispatcher =
      PermissionRequestDispatcher::FromWebContents(web_contents);
  if (!dispatcher) {
    // Are there any cases when this can be null?
    callback.Run(content::PermissionStatus::DENIED);
    return kNoPendingOperation;
  }

  if (!dispatcher->CanDispatchRequest()) {
    callback.Run(content::PermissionStatus::DENIED);
    return kNoPendingOperation;
  }

  RequestData* request_data = new RequestData(render_frame_host);
  int request_id = requests_.Add(request_data);

  request_data->request_id = dispatcher->RequestPermission(
      permission,
      render_frame_host,
      requesting_origin,
      base::Bind(&PermissionManager::RespondToPermissionRequest,
                 weak_factory_.GetWeakPtr(),
                 request_id,
                 permission,
                 requesting_origin,
                 embedding_origin,
                 callback));
  DCHECK_NE(request_data->request_id, -1);

  return request_id;
}

int PermissionManager::RequestPermissions(
    const std::vector<content::PermissionType>& permissions,
    content::RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    const base::Callback<void(
        const std::vector<content::PermissionStatus>&)>& callback) {
  NOTIMPLEMENTED();

  std::vector<content::PermissionStatus> result(permissions.size());
  const GURL& embedding_origin =
      content::WebContents::FromRenderFrameHost(render_frame_host)
          ->GetLastCommittedURL().GetOrigin();

  for (content::PermissionType type : permissions) {
    result.push_back(GetPermissionStatus(
        type, requesting_origin, embedding_origin));
  }

  callback.Run(result);
  return kNoPendingOperation;
}

void PermissionManager::CancelPermissionRequest(int request_id) {
  RequestData* data = requests_.Lookup(request_id);

  int dispatcher_request_id = data->request_id;
  RenderFrameHostID render_frame_host_id = data->render_frame_host_id;

  requests_.Remove(request_id);

  content::RenderFrameHost* render_frame_host =
      render_frame_host_id.ToInstance();
  if (!render_frame_host) {
    return;
  }

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

  dispatcher->CancelPermissionRequest(dispatcher_request_id);
}

content::PermissionStatus PermissionManager::GetPermissionStatus(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
  if (!IsPermissionTypeSupported(permission)) {
    return content::PermissionStatus::DENIED;
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
    : context_(context),
      weak_factory_(this) {}

PermissionManager::~PermissionManager() {
  DCHECK(subscriptions_.IsEmpty());
  DCHECK(requests_.IsEmpty());
}

} // namespace oxide
