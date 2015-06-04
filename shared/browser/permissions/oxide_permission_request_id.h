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

#ifndef _OXIDE_SHARED_BROWSER_PERMISSIONS_PERMISSION_REQUEST_ID_H_
#define _OXIDE_SHARED_BROWSER_PERMISSIONS_PERMISSION_REQUEST_ID_H_

#include "url/gurl.h"

namespace oxide {

// Request ID based on PermissionRequestID in Chromium. It is required for
// requests that want to participate in cancellation. The design is a bit
// weird though, as |bridge_id| appears to be a per-frame ID from Chromium,
// making it possible for requests from different frames with the same origin
// to have the same ID
class PermissionRequestID {
 public:
  PermissionRequestID(int render_process_id,
                      int render_view_id,
                      int bridge_id,
                      const GURL& origin);

  // Constructs an invalid ID
  PermissionRequestID();

  ~PermissionRequestID();

  // Whether this is a valid ID
  bool IsValid() const;

  bool operator==(const PermissionRequestID& other) const;
  bool operator!=(const PermissionRequestID& other) const {
    return !(*this == other);
  }

 private:
  int render_process_id_;
  int render_view_id_;
  int bridge_id_;
  GURL origin_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_PERMISSIONS_PERMISSION_REQUEST_ID_H_
