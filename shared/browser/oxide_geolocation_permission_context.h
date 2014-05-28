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

#ifndef _OXIDE_SHARED_BROWSER_GEOLOCATION_PERMISSION_CONTEXT_
#define _OXIDE_SHARED_BROWSER_GEOLOCATION_PERMISSION_CONTEXT_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/public/browser/geolocation_permission_context.h"

namespace oxide {

class GeolocationPermissionContext FINAL :
    public content::GeolocationPermissionContext {
 public:
  GeolocationPermissionContext();

  void RequestGeolocationPermission(
      content::WebContents* contents,
      int bridge_id,
      const GURL& requesting_frame,
      bool user_gesture,
      base::Callback<void(bool)> callback) FINAL;

  void CancelGeolocationPermissionRequest(
      content::WebContents* contents,
      int bridge_id,
      const GURL& requesting_frame) FINAL;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_GEOLOCATION_PERMISSION_CONTEXT_
