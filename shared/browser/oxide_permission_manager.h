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
#include "base/macros.h"
#include "content/public/browser/permission_manager.h"

namespace oxide {

class PermissionManager : public content::PermissionManager {
 public:
  PermissionManager();
  ~PermissionManager() override;

 private:
  // content::PermissionManager implementation
  void RequestPermission(
      content::PermissionType permission,
      content::WebContents* web_contents,
      int request_id,
      const GURL& requesting_origin,
      bool user_gesture,
      const base::Callback<void(content::PermissionStatus)>& callback) override;
  void CancelPermissionRequest(content::PermissionType permission,
                               content::WebContents* web_contents,
                               int request_id,
                               const GURL& requesting_origin) override;
  content::PermissionStatus GetPermissionStatus(
      content::PermissionType permission,
      const GURL& requesting_origin,
      const GURL& embedding_origin) override;
  void ResetPermission(content::PermissionType permission,
                       const GURL& requesting_origin,
                       const GURL& embedding_origin) override;
  void RegisterPermissionUsage(content::PermissionType permission,
                               const GURL& requesting_origin,
                               const GURL& embedding_origin) override;

  DISALLOW_COPY_AND_ASSIGN(PermissionManager);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_PERMISSION_MANAGER_H_
