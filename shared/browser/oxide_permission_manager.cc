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

#include "oxide_web_view.h"

namespace oxide {

namespace {

void RespondToGeolocationPermissionRequest(
    const base::Callback<void(content::PermissionStatus)>& callback,
    content::PermissionStatus result) {
  if (result == content::PERMISSION_STATUS_GRANTED) {
    content::GeolocationProvider::GetInstance()
        ->UserDidOptIntoLocationServices();
  }
  callback.Run(result);
}

}

struct PermissionManager::Subscription {
  base::Callback<void(content::PermissionStatus)> callback;
};

void PermissionManager::RequestPermission(
    content::PermissionType permission,
    content::WebContents* web_contents,
    int request_id,
    const GURL& requesting_origin,
    bool user_gesture,
    const base::Callback<void(content::PermissionStatus)>& callback) {
  if (permission != content::PermissionType::GEOLOCATION) {
    // TODO: Other types
    callback.Run(content::PERMISSION_STATUS_DENIED);
    return;
  }

  WebView* webview = WebView::FromWebContents(web_contents);
  if (!webview) {
    callback.Run(content::PERMISSION_STATUS_DENIED);
    return;
  }

  base::Callback<void(content::PermissionStatus)> wrapped_callback =
      base::Bind(&RespondToGeolocationPermissionRequest,
                 callback);
  webview->RequestGeolocationPermission(requesting_origin,
                                        request_id,
                                        wrapped_callback);
}

void PermissionManager::CancelPermissionRequest(
    content::PermissionType permission,
    content::WebContents* web_contents,
    int request_id,
    const GURL& requesting_origin) {
  if (permission != content::PermissionType::GEOLOCATION) {
    return;
  }

  WebView* webview = WebView::FromWebContents(web_contents);
  if (!webview) {
    return;
  }

  webview->CancelGeolocationPermissionRequest(requesting_origin,
                                              request_id);
}

content::PermissionStatus PermissionManager::GetPermissionStatus(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
  return content::PERMISSION_STATUS_DENIED;
}

void PermissionManager::ResetPermission(content::PermissionType permission,
                                        const GURL& requesting_origin,
                                        const GURL& embedding_origin) {}

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

PermissionManager::PermissionManager() {}

PermissionManager::~PermissionManager() {}

} // namespace oxide
