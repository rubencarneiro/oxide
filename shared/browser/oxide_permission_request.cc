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

#include "oxide_permission_request.h"

#include <algorithm>

#include "base/logging.h"

namespace oxide {

PermissionRequest::ID::ID(int render_process_id,
                          int render_view_id,
                          int request_id)
    : render_process_id_(render_process_id),
      render_view_id_(render_view_id),
      request_id_(request_id) {}

bool PermissionRequest::ID::Equals(const ID& other) {
  return render_process_id_ == other.render_process_id_ &&
         render_view_id_ == other.render_view_id_ &&
         request_id_ == other.request_id_;
}

bool PermissionRequest::ID::operator==(const ID& other) {
  return Equals(other);
}

void PermissionRequest::SetDidRespond() {
  DCHECK(!is_cancelled_);
  did_respond_ = true;
  if (manager_) {
    manager_->RemovePendingPermissionRequest(this);
  }
}

PermissionRequest::PermissionRequest(PermissionRequestManager* manager,
                                     const ID& id,
                                     const GURL& origin,
                                     const GURL& embedder)
    : manager_(manager->AsWeakPtr()),
      id_(id),
      origin_(origin),
      embedder_(embedder),
      is_cancelled_(false),
      did_respond_(false) {
  manager_->AddPendingPermissionRequest(this);
}

PermissionRequest::~PermissionRequest() {
  // We remove ourself from PermissionRequestManager when setting either
  // of these, so assert that this has happened to prevent a dangling
  // pointer
  CHECK(is_cancelled_ || did_respond_);
}

void PermissionRequest::Cancel() {
  DCHECK(!did_respond_ && !is_cancelled_);
  is_cancelled_ = true;
  if (manager_) {
    manager_->RemovePendingPermissionRequest(this);
  }
  if (!cancel_callback_.is_null()) {
    cancel_callback_.Run();
  }
}

void PermissionRequest::SetCancelCallback(const base::Closure& callback) {
  cancel_callback_ = callback;
  if (is_cancelled_) {
    cancel_callback_.Run();
  }
}

void PermissionRequestManager::AddPendingPermissionRequest(
    PermissionRequest* request) {
  for (PermissionRequestVector::iterator it = pending_requests_.begin();
       it != pending_requests_.end(); ++it) {
    DCHECK(!(*it)->id().Equals(request->id()));
  }

  pending_requests_.push_back(request);
}

void PermissionRequestManager::RemovePendingPermissionRequest(
    PermissionRequest* request) {
  PermissionRequestVector::iterator it =
      std::find(pending_requests_.begin(), pending_requests_.end(), request);
  if (it != pending_requests_.end()) {
    if (in_dispatch_) {
      *it = NULL;
    } else {
      pending_requests_.erase(it);
    }
  }
}

void PermissionRequestManager::Compact() {
  DCHECK(!in_dispatch_);
  pending_requests_.erase(
      std::remove(pending_requests_.begin(),
                  pending_requests_.end(),
                  static_cast<PermissionRequest *>(NULL)),
      pending_requests_.end());
}

PermissionRequestManager::PermissionRequestManager()
    : in_dispatch_(false) {}

PermissionRequestManager::~PermissionRequestManager() {
  CancelAllPending();
}

void PermissionRequestManager::CancelAllPending() {
  DCHECK(!in_dispatch_);
  in_dispatch_ = true;

  for (PermissionRequestVector::iterator it = pending_requests_.begin();
       it != pending_requests_.end(); ++it) {
    PermissionRequest* request = *it;
    if (request) {
      request->Cancel();
    }
  }

  in_dispatch_ = false;
  Compact();
}

void PermissionRequestManager::CancelPendingRequestWithID(
    const PermissionRequest::ID& id) {
  for (PermissionRequestVector::iterator it = pending_requests_.begin();
       it != pending_requests_.end(); ++it) {
    PermissionRequest* request = *it;
    if (request->id().Equals(id)) {
      request->Cancel();
      break;
    }
  }
}

GeolocationPermissionRequest::GeolocationPermissionRequest(
    PermissionRequestManager* manager,
    const PermissionRequest::ID& id,
    const GURL& origin,
    const GURL& embedder,
    const base::Callback<void(bool)>& callback)
    : PermissionRequest(manager, id, origin, embedder),
      callback_(callback) {}

GeolocationPermissionRequest::~GeolocationPermissionRequest() {
  if (!did_respond()) {
    Deny();
  }
}

void GeolocationPermissionRequest::Accept() {
  DCHECK(!did_respond() && !is_cancelled());
  SetDidRespond();
  callback_.Run(true);
}

void GeolocationPermissionRequest::Deny() {
  DCHECK(!did_respond() && !is_cancelled());
  SetDidRespond();
  callback_.Run(false);
}

} // namespace oxide
