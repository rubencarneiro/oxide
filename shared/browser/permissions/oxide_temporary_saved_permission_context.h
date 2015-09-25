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

#ifndef _OXIDE_SHARED_BROWSER_PERMISSIONS_TEMPORARY_SAVED_PERMISSION_CONTEXT_H_
#define _OXIDE_SHARED_BROWSER_PERMISSIONS_TEMPORARY_SAVED_PERMISSION_CONTEXT_H_

#include <map>
#include <utility>

#include "base/macros.h"
#include "base/synchronization/lock.h"
#include "url/gurl.h"

namespace content {
class BrowserContext;
}

namespace oxide {

enum TemporarySavedPermissionStatus {
  TEMPORARY_SAVED_PERMISSION_STATUS_DENIED,
  TEMPORARY_SAVED_PERMISSION_STATUS_ALLOWED,
  TEMPORARY_SAVED_PERMISSION_STATUS_ASK
};

enum TemporarySavedPermissionType {
  PERMISSION_TYPES_START,

  TEMPORARY_SAVED_PERMISSION_TYPE_GEOLOCATION = PERMISSION_TYPES_START,
  TEMPORARY_SAVED_PERMISSION_TYPE_MEDIA_DEVICE_MIC,
  TEMPORARY_SAVED_PERMISSION_TYPE_MEDIA_DEVICE_CAMERA,
  TEMPORARY_SAVED_PERMISSION_TYPE_NOTIFICATIONS,

  NUM_PERMISSION_TYPES
};

// This is a per-BrowserContext object for storing permission request
// decisions for the rest of a browsing session.
// XXX: This class is only here temporarily (hence the name
//  "TemporarySavedPermissionContext") and will be removed in favour of content
//  settings in the future, which will be used for permanent and session
//  persistence of permission request decisions (as well as other stuff).
//  Please don't add new features to this class, other than expanding
//  TemporarySavedPermissionType
class TemporarySavedPermissionContext {
 public:
  TemporarySavedPermissionContext();
  ~TemporarySavedPermissionContext();

  TemporarySavedPermissionStatus GetPermissionStatus(
      TemporarySavedPermissionType type,
      const GURL& primary_url,
      const GURL& secondary_url);

  void SetPermissionStatus(TemporarySavedPermissionType type,
                           const GURL& primary_url,
                           const GURL& secondary_url,
                           TemporarySavedPermissionStatus status);

  void Clear();

 private:
  base::Lock lock_;

  typedef std::pair<GURL, GURL> Key;
  typedef std::map<Key, TemporarySavedPermissionStatus> Map;

  Map maps_[NUM_PERMISSION_TYPES];

  DISALLOW_COPY_AND_ASSIGN(TemporarySavedPermissionContext);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_PERMISSIONS_TEMPORARY_SAVED_PERMISSION_CONTEXT_H_
