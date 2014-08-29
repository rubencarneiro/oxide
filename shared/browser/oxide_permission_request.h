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

#ifndef _OXIDE_SHARED_BROWSER_PERMISSION_REQUEST_H_
#define _OXIDE_SHARED_BROWSER_PERMISSION_REQUEST_H_

#include <vector>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "url/gurl.h"

namespace oxide {

enum PermissionRequestType {
  PERMISSION_REQUEST_TYPE_START = 0,

  PERMISSION_REQUEST_TYPE_GEOLOCATION = PERMISSION_REQUEST_TYPE_START,

  PERMISSION_REQUEST_TYPE_MAX
};

class PermissionRequest;
class SimplePermissionRequest;

class PermissionRequestManager FINAL :
    public base::SupportsWeakPtr<PermissionRequestManager> {
 public:
  PermissionRequestManager();
  ~PermissionRequestManager();

  scoped_ptr<SimplePermissionRequest> CreateGeolocationPermissionRequest(
      const GURL& origin,
      const GURL& embedder,
      const base::Callback<void(bool)>& callback,
      base::Closure* cancel_callback);

  void CancelAllPendingRequests();
  void CancelAllPendingRequestsForType(PermissionRequestType type);

 private:
  friend class PermissionRequest;
  typedef std::vector<PermissionRequest *> PermissionRequestVector;

  static void CancelPendingRequest(
      const base::WeakPtr<PermissionRequest>& request);

  void AddPendingRequest(PermissionRequestType type,
                         PermissionRequest* request);
  void RemovePendingRequest(PermissionRequest* request);

  bool in_dispatch_;
  PermissionRequestVector pending_requests_[PERMISSION_REQUEST_TYPE_MAX];

  DISALLOW_COPY_AND_ASSIGN(PermissionRequestManager);
};

class PermissionRequest : public base::SupportsWeakPtr<PermissionRequest> {
 public:
  virtual ~PermissionRequest();

  GURL origin() const { return origin_; }
  GURL embedder() const { return embedder_; }

  // Sets a callback to run if the request is cancelled by Oxide
  void SetCancelCallback(const base::Closure& cancel_callback);

 protected:
  PermissionRequest(const GURL& origin,
                    const GURL& embedder);

  // Called by Oxide to cancel this request. Will notify the callback
  // registered with SetCancelCallback
  virtual void Cancel();

 private:
  friend class PermissionRequestManager;

  PermissionRequestType type_;
  base::WeakPtr<PermissionRequestManager> manager_;
  GURL origin_;
  GURL embedder_;

  bool is_cancelled_;
  base::Closure cancel_callback_;

  DISALLOW_COPY_AND_ASSIGN(PermissionRequest);
};

class SimplePermissionRequest FINAL : public PermissionRequest {
 public:
  ~SimplePermissionRequest();

  void Allow();
  void Deny();

 private:
  friend class PermissionRequestManager;

  SimplePermissionRequest(const GURL& origin,
                          const GURL& embedder,
                          const base::Callback<void(bool)>& callback);

  // Called by Oxide to cancel this request. Once called, the API layer
  // must not call Allow() or Deny()
  void Cancel() FINAL;

  base::Callback<void(bool)> callback_;

  DISALLOW_COPY_AND_ASSIGN(SimplePermissionRequest);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_PERMISSION_REQUEST_H_
