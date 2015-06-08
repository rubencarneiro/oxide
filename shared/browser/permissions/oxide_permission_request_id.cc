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

#include "oxide_permission_request_id.h"

namespace oxide {

PermissionRequestID::PermissionRequestID(int render_process_id,
                                         int render_view_id,
                                         int bridge_id,
                                         const GURL& origin)
    : render_process_id_(render_process_id),
      render_view_id_(render_view_id),
      bridge_id_(bridge_id),
      origin_(origin) {}

PermissionRequestID::PermissionRequestID()
    : render_process_id_(-1),
      render_view_id_(-1),
      bridge_id_(-1) {}

PermissionRequestID::~PermissionRequestID() {}

bool PermissionRequestID::IsValid() const {
  return render_process_id_ > -1 && render_view_id_ > -1 && bridge_id_ > -1;
}

bool PermissionRequestID::operator==(const PermissionRequestID& other) const {
  return render_process_id_ == other.render_process_id_ &&
         render_view_id_ == other.render_view_id_ &&
         bridge_id_ == other.bridge_id_ &&
         origin_ == other.origin_;
}

} // namespace oxide
