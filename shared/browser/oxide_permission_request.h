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
#include "base/memory/weak_ptr.h"
#include "url/gurl.h"

namespace oxide {

class PermissionRequestManager;

class PermissionRequest {
 public:
  PermissionRequest(PermissionRequestManager* manager,
                    int id, const GURL& origin,
                    const GURL& embedder);
  virtual ~PermissionRequest();

  int id() const { return id_; }
  GURL origin() const { return origin_; }
  GURL embedder() const { return embedder_; }

  void Cancel();
  bool is_cancelled() const { return is_cancelled_; }

  void SetCancelCallback(const base::Closure& cancel_callback);

  bool did_respond() const { return did_respond_; }

 protected:
  void SetDidRespond();

 private:
  base::WeakPtr<PermissionRequestManager> manager_;
  int id_;
  GURL origin_;
  GURL embedder_;

  base::Closure cancel_callback_;
  bool is_cancelled_;

  bool did_respond_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(PermissionRequest);
};

class PermissionRequestManager FINAL :
    public base::SupportsWeakPtr<PermissionRequestManager> {
 public:
  PermissionRequestManager();
  ~PermissionRequestManager();

  void CancelAllPending();
  void CancelPendingRequestWithID(int id);

 private:
  friend class PermissionRequest;
  typedef std::vector<PermissionRequest *> PermissionRequestVector;

  void AddPendingPermissionRequest(PermissionRequest* request);
  void RemovePendingPermissionRequest(PermissionRequest* request);

  void Compact();

  bool in_dispatch_;
  PermissionRequestVector pending_requests_;

  DISALLOW_COPY_AND_ASSIGN(PermissionRequestManager);
};

class GeolocationPermissionRequest FINAL : public PermissionRequest {
 public:
  GeolocationPermissionRequest(PermissionRequestManager* manager,
                               int id,
                               const GURL& origin,
                               const GURL& embedder,
                               const base::Callback<void(bool)>& callback);
  ~GeolocationPermissionRequest();

  void Accept();
  void Deny();

 private:
  base::Callback<void(bool)> callback_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(GeolocationPermissionRequest);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_PERMISSION_REQUEST_H_
