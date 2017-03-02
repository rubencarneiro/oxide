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

#ifndef _OXIDE_SHARED_BROWSER_PERMISSION_MANAGER_H_
#define _OXIDE_SHARED_BROWSER_PERMISSION_MANAGER_H_

#include "base/callback.h"
#include "base/id_map.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/permission_manager.h"

#include "shared/browser/permissions/oxide_permission_request_response.h"

namespace oxide {

class BrowserContext;

class PermissionManager : public content::PermissionManager {
 public:
  PermissionManager(BrowserContext* context);
  ~PermissionManager() override;

 private:
  void RespondToPermissionRequest(
      int request_id,
      content::PermissionType permission,
      const GURL& requesting_origin,
      const GURL& embedding_origin,
      const base::Callback<void(blink::mojom::PermissionStatus)>& callback,
      PermissionRequestResponse response);

  // content::PermissionManager implementation
  int RequestPermission(
      content::PermissionType permission,
      content::RenderFrameHost* render_frame_host,
      const GURL& requesting_origin,
      bool user_gesture,
      const base::Callback<
          void(blink::mojom::PermissionStatus)>& callback) override;
  int RequestPermissions(
      const std::vector<content::PermissionType>& permissions,
      content::RenderFrameHost* render_frame_host,
      const GURL& requesting_origin,
      bool user_gesture,
      const base::Callback<
          void(const std::vector<blink::mojom::PermissionStatus>&)>& callback) override;
  void CancelPermissionRequest(int request_id) override;
  blink::mojom::PermissionStatus GetPermissionStatus(
      content::PermissionType permission,
      const GURL& requesting_origin,
      const GURL& embedding_origin) override;
  void ResetPermission(content::PermissionType permission,
                       const GURL& requesting_origin,
                       const GURL& embedding_origin) override;
  int SubscribePermissionStatusChange(
      content::PermissionType permission,
      const GURL& requesting_origin,
      const GURL& embedding_origin,
      const base::Callback<
          void(blink::mojom::PermissionStatus)>& callback) override;
  void UnsubscribePermissionStatusChange(int subscription_id) override;

  BrowserContext* context_; // We're owned by |context_|

  struct Subscription;
  IDMap<std::unique_ptr<Subscription>> subscriptions_;

  struct RequestData;
  IDMap<std::unique_ptr<RequestData>> requests_;

  base::WeakPtrFactory<PermissionManager> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(PermissionManager);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_PERMISSION_MANAGER_H_
