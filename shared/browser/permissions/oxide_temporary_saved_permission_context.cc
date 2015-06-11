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

#include "oxide_temporary_saved_permission_context.h"

namespace oxide {

TemporarySavedPermissionContext::TemporarySavedPermissionContext() {}

TemporarySavedPermissionContext::~TemporarySavedPermissionContext() {}

TemporarySavedPermissionStatus
TemporarySavedPermissionContext::GetPermissionStatus(
    TemporarySavedPermissionType type,
    const GURL& primary_url,
    const GURL& secondary_url) {
  Key key = std::make_pair(primary_url, secondary_url);

  base::AutoLock lock(lock_);

  Map& map = maps_[type];
  const auto& it = map.find(key);
  if (it == map.end()) {
    return TEMPORARY_SAVED_PERMISSION_STATUS_ASK;
  }

  return it->second;
}

void TemporarySavedPermissionContext::SetPermissionStatus(
    TemporarySavedPermissionType type,
    const GURL& primary_url,
    const GURL& secondary_url,
    TemporarySavedPermissionStatus status) {
  Key key = std::make_pair(primary_url, secondary_url);

  base::AutoLock lock(lock_);

  Map& map = maps_[type];
  if (status == TEMPORARY_SAVED_PERMISSION_STATUS_ASK) {
    map.erase(key);
    return;
  }

  map[key] = status;
}

void TemporarySavedPermissionContext::Clear() {
  base::AutoLock lock(lock_);
  for (int i = PERMISSION_TYPES_START; i < NUM_PERMISSION_TYPES; ++i) {
    maps_[i].clear();
  }
}

} // namespace oxide
