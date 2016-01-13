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

#ifndef _OXIDE_SHARED_BROWSER_PERMISSION_REQUEST_H_
#define _OXIDE_SHARED_BROWSER_PERMISSION_REQUEST_H_

#include "base/macros.h"
#include "url/gurl.h"

#include "shared/browser/oxide_render_object_id.h"
#include "shared/browser/permissions/oxide_permission_request_dispatcher.h"
#include "shared/browser/permissions/oxide_permission_request_response.h"

namespace oxide {

// Base class of all PermissionRequests. It contains functionality that is
// common to all requests (url, embedder, allow, deny). If your request
// requires more information to be exposed, feel free to subclass from this 
class PermissionRequest {
 public:
  PermissionRequest(int request_id,
                    const RenderFrameHostID& frame_id,
                    const GURL& origin,
                    const GURL& embedder,
                    const PermissionRequestCallback& callback);
  virtual ~PermissionRequest();

  // The origin of the frame that generated this request
  GURL origin() const { return origin_; }

  // The origin of the top-level document containing the frame that generated
  // this request
  GURL embedder() const { return embedder_; }

  // Whether this request is still waiting for a response
  bool IsPending() const;

  // Whether this request has been cancelled
  bool is_cancelled() const { return is_cancelled_; }

  // Set a callback to be invoked when this request is cancelled
  void SetCancelCallback(const base::Closure& cancel_callback);

  // Allow the requesting frame access to the desired resource
  void Allow();

  // Deny the requesting frame access to the desired resource
  void Deny();

 private:
  friend class PermissionRequestDispatcher;

  // Cancel this request and run the cancel callback. This is only called from
  // PermissionRequestDispatcher or this classes destructor
  void Cancel(bool run_callback);

  void Respond(PermissionRequestResponse response);

  // The unique ID of this request - used for cancellation from Chromium via
  // PermissionManager
  int request_id_;

  // The ID of the RenderFrameHost that initiated this request
  RenderFrameHostID frame_id_;

  PermissionRequestDispatcher* dispatcher_;

  GURL origin_;
  GURL embedder_;

  bool is_cancelled_;
  base::Closure cancel_callback_;

  // The callback provided by Chromium, which we use to respond to the request
  PermissionRequestCallback callback_;

  DISALLOW_COPY_AND_ASSIGN(PermissionRequest);
};

class MediaAccessPermissionRequest : public PermissionRequest {
 public:
  MediaAccessPermissionRequest(int request_id,
                               const RenderFrameHostID& frame_id,
                               const GURL& origin,
                               const GURL& embedder,
                               bool audio_requested,
                               bool video_requested,
                               const PermissionRequestCallback& callback);
  ~MediaAccessPermissionRequest() override;

  bool audio_requested() const { return audio_requested_; }

  bool video_requested() const { return video_requested_; }

 private:
  bool audio_requested_;
  bool video_requested_;

  DISALLOW_COPY_AND_ASSIGN(MediaAccessPermissionRequest);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_PERMISSION_REQUEST_H_
