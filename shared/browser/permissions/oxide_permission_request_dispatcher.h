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

#ifndef _OXIDE_SHARED_BROWSER_PERMISSIONS_PERMISSION_REQUEST_DISPATCHER_H_
#define _OXIDE_SHARED_BROWSER_PERMISSIONS_PERMISSION_REQUEST_DISPATCHER_H_

#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/permission_type.h"
#include "content/public/browser/web_contents_user_data.h"

#include "shared/browser/permissions/oxide_permission_request_response.h"

class GURL;

namespace content {
class RenderFrameHost;
class WebContents;
}

namespace oxide {

typedef base::Callback<void(PermissionRequestResponse)>
    PermissionRequestCallback;

class PermissionRequest;
class PermissionRequestDispatcherClient;
class WebFrame;

// This class keeps track of pending permission requests for a WebContents
// instance
// TODO: Coalesce requests with the same embedder/request origin and type
class PermissionRequestDispatcher
    : public content::WebContentsUserData<PermissionRequestDispatcher> {
 public:
  ~PermissionRequestDispatcher();

  void set_client(PermissionRequestDispatcherClient* client) {
    client_ = client;
  }

  // Whether we can dispatch a request to the client
  bool CanDispatchRequest() const;

  // Request permission to use the resource identified by |permission|
  int RequestPermission(content::PermissionType permission,
                        content::RenderFrameHost* render_frame_host,
                        const GURL& requesting_origin,
                        const PermissionRequestCallback& callback);

  // Cancel the pending permission request
  void CancelPermissionRequest(int request_id);

  // Request permission to access media devices
  int RequestMediaAccessPermission(content::RenderFrameHost* render_frame_host,
                                   const GURL& requesting_origin,
                                   bool audio,
                                   bool video,
                                   const PermissionRequestCallback& callback);

  // Cancel any pending permission requests
  void CancelPendingRequests();

  // Cancel any pending permission requests for |frame|
  void CancelPendingRequestsForFrame(WebFrame* frame);

 private:
  friend class content::WebContentsUserData<PermissionRequestDispatcher>;
  friend class PermissionRequest;
  class IteratorGuard;

  PermissionRequestDispatcher(content::WebContents* contents);

  // Add a PermissionRequest to this manager
  void AddPendingRequest(PermissionRequest* request);

  // Remove |request| from this manager and invalidate its pointer to this
  void RemovePendingRequest(PermissionRequest* request);

  // Remove empty slots from pending_requests_
  void Compact();

  content::WebContents* contents_;

  PermissionRequestDispatcherClient* client_;

  // Used to indicate that pending_requests_ is being iterated over, and
  // will prevent RemovePendingRequest from mutating it
  bool iterating_;

  typedef std::vector<PermissionRequest*> PermissionRequestVector;

  // This list of PermissionRequests
  std::vector<PermissionRequest*> pending_requests_;

  int next_request_id_;

  base::WeakPtrFactory<PermissionRequestDispatcher> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(PermissionRequestDispatcher);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_PERMISSIONS_PERMISSION_REQUEST_DISPATCHER_H_
