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
void PermissionRequestManager::CancelPendingRequestFromSource(
    const base::WeakPtr<PermissionRequest>& request) {
  if (request) {
    request->Cancel(true);
  }
}

void PermissionRequestManager::AddPendingRequest(PermissionRequestType type,
                                                 PermissionRequest* request) {
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
  CHECK(!HasAnyPendingRequests());
}

scoped_ptr<SimplePermissionRequest>
PermissionRequestManager::CreateSimplePermissionRequest(
    PermissionRequestType type,
    const base::Callback<void(bool)>& callback,
    base::Closure* cancel_callback) {
  CHECK_LT(type, PERMISSION_REQUEST_TYPE_MAX);

  scoped_ptr<SimplePermissionRequest> rv(
      new SimplePermissionRequest(callback));
  AddPendingRequest(type, rv.get());

  if (cancel_callback) {
    *cancel_callback = base::Bind(CancelPendingRequestFromSource,
                                  rv->AsWeakPtr());
  }

  return rv.Pass();
}

bool PermissionRequestManager::HasAnyPendingRequests() {
  for (int i = PERMISSION_REQUEST_TYPE_START;
       i < PERMISSION_REQUEST_TYPE_MAX; ++i) {
    if (HasPendingRequestsForType(static_cast<PermissionRequestType>(i))) {
      return true;
    }
  }

  return false;
}

bool PermissionRequestManager::HasPendingRequestsForType(
    PermissionRequestType type) {
  CHECK_LT(type, PERMISSION_REQUEST_TYPE_MAX);

  PermissionRequestVector& requests = pending_requests_[type];

  for (PermissionRequestVector::iterator it = requests.begin();
       it != requests.end(); ++it) {
    PermissionRequest* req = *it;
    if (!req->is_cancelled_ && req->CanRespond()) {
      return true;
    }
  }

  return false;
}

void PermissionRequestManager::AbortPendingRequest(
    PermissionRequest* request) {
  if (!request) {
    return;
  }

  request->Cancel(true);
}

void PermissionRequestManager::CancelAllPendingRequests() {
  for (int i = PERMISSION_REQUEST_TYPE_START;
       i < PERMISSION_REQUEST_TYPE_MAX; ++i) {
    CancelPendingRequestsForType(static_cast<PermissionRequestType>(i));
  }
}

void PermissionRequestManager::CancelPendingRequestsForType(
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
        request->Cancel(false);
      }
    }
  }

  requests.erase(
      std::remove(requests.begin(), requests.end(),
                  static_cast<PermissionRequest *>(NULL)),
      requests.end());
}

PermissionRequest::PermissionRequest()
    : type_(PERMISSION_REQUEST_TYPE_START),
      is_cancelled_(false) {}

void PermissionRequest::Cancel(bool from_source) {
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
    const base::Callback<void(bool)>& callback)
    : callback_(callback) {}

void SimplePermissionRequest::Cancel(bool from_source) {
  if (callback_.is_null()) {
    return;
  }
  if (!from_source && !callback_.is_null()) {
    Deny();
  }
  callback_.Reset();
  PermissionRequest::Cancel(from_source);
}

bool SimplePermissionRequest::CanRespond() const {
  return !callback_.is_null();
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
