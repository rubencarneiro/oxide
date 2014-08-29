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

#include "base/auto_reset.h"
#include "base/bind.h"
#include "base/logging.h"

namespace oxide {

// static
void PermissionRequestManager::CancelPendingRequest(
    const base::WeakPtr<PermissionRequest>& request) {
  if (request) {
    request->Cancel();
  }
}

void PermissionRequestManager::AddPendingRequest(PermissionRequestType type,
                                                 PermissionRequest* request) {
  DCHECK_LT(type, PERMISSION_REQUEST_TYPE_MAX);
  PermissionRequestVector& requests = pending_requests_[type];

  DCHECK(std::find(requests.begin(), requests.end(), request) == requests.end());

  request->type_ = type;
  request->manager_ = AsWeakPtr();
  requests.push_back(request);
}

void PermissionRequestManager::RemovePendingRequest(
    PermissionRequest* request) {
  DCHECK_LT(request->type_, PERMISSION_REQUEST_TYPE_MAX);

  PermissionRequestVector& requests = pending_requests_[request->type_];

  PermissionRequestVector::iterator it =
      std::find(requests.begin(), requests.end(), request);
  CHECK(it != requests.end());
  if (in_dispatch_) {
    *it = NULL;
  } else {
    requests.erase(it);
  }
}

PermissionRequestManager::PermissionRequestManager()
    : in_dispatch_(false) {}

PermissionRequestManager::~PermissionRequestManager() {
  CancelAllPendingRequests();
}

scoped_ptr<SimplePermissionRequest>
PermissionRequestManager::CreateGeolocationPermissionRequest(
    const GURL& origin,
    const GURL& embedder,
    const base::Callback<void(bool)>& callback,
    base::Closure* cancel_callback) {
  scoped_ptr<SimplePermissionRequest> rv(
      new SimplePermissionRequest(origin, embedder, callback));
  AddPendingRequest(PERMISSION_REQUEST_TYPE_GEOLOCATION,
                              rv.get());

  if (cancel_callback) {
    *cancel_callback = base::Bind(CancelPendingRequest,
                                  rv->AsWeakPtr());
  }

  return rv.Pass();
}

void PermissionRequestManager::CancelAllPendingRequests() {
  for (int i = PERMISSION_REQUEST_TYPE_START;
       i < PERMISSION_REQUEST_TYPE_MAX; ++i) {
    CancelAllPendingRequestsForType(static_cast<PermissionRequestType>(i));
  }
}

void PermissionRequestManager::CancelAllPendingRequestsForType(
    PermissionRequestType type) {
  CHECK_LT(type, PERMISSION_REQUEST_TYPE_MAX);
  DCHECK(!in_dispatch_);

  PermissionRequestVector& requests = pending_requests_[type];

  {
    base::AutoReset<bool> d(&in_dispatch_, true);

    for (PermissionRequestVector::iterator it = requests.begin();
         it != requests.end(); ++it) {
      PermissionRequest* request = *it;
      if (request) {
        request->Cancel();
      }
    }
  }

  requests.erase(
      std::remove(requests.begin(), requests.end(),
                  static_cast<PermissionRequest *>(NULL)),
      requests.end());
}

PermissionRequest::PermissionRequest(const GURL& origin,
                                     const GURL& embedder)
    : type_(PERMISSION_REQUEST_TYPE_START),
      origin_(origin),
      embedder_(embedder),
      is_cancelled_(false) {}

void PermissionRequest::Cancel() {
  if (is_cancelled_) {
    // Can be called multiple times from PermissionRequestManager
    return;
  }
  is_cancelled_ = true;
  if (!cancel_callback_.is_null()) {
    cancel_callback_.Run();
  }
}

PermissionRequest::~PermissionRequest() {
  if (manager_) {
    manager_->RemovePendingRequest(this);
  }
}

void PermissionRequest::SetCancelCallback(const base::Closure& callback) {
  DCHECK(!is_cancelled_);
  cancel_callback_ = callback;
}

SimplePermissionRequest::SimplePermissionRequest(
    const GURL& origin,
    const GURL& embedder,
    const base::Callback<void(bool)>& callback)
    : PermissionRequest(origin, embedder),
      callback_(callback) {}

void SimplePermissionRequest::Cancel() {
  if (callback_.is_null()) {
    return;
  }
  callback_.Reset();
  PermissionRequest::Cancel();
}

SimplePermissionRequest::~SimplePermissionRequest() {
  if (!callback_.is_null()) {
    Deny();
  }
}

void SimplePermissionRequest::Allow() {
  if (callback_.is_null()) {
    LOG(ERROR) << "Cannot Allow() a request that has been cancelled or "
                  "responded to already";
    return;
  }

  callback_.Run(true);
  callback_.Reset();
}

void SimplePermissionRequest::Deny() {
  if (callback_.is_null()) {
    LOG(ERROR) << "Cannot Deny() a request that has been cancelled or "
                  "responded to already";
    return;
  }

  callback_.Run(false);
  callback_.Reset();
}

} // namespace oxide
