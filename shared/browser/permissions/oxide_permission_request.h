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

#include "base/callback.h"
#include "base/macros.h"
#include "content/public/common/media_stream_request.h"
#include "content/public/common/permission_status.mojom.h"
#include "url/gurl.h"

#include "shared/browser/permissions/oxide_permission_request_id.h"

namespace oxide {

class PermissionRequestDispatcher;
class WebFrame;

// Base class of all PermissionRequests - this shouldn't be used directly.
// It contains functionality that is common to all requests (url, embedder)
class PermissionRequest {
 public:
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

 protected:
  friend class PermissionRequestDispatcher;

  PermissionRequest(const PermissionRequestID& request_id,
                    WebFrame* frame,
                    const GURL& origin,
                    const GURL& embedder);

  // Cancel this request and run the cancel callback. This is only called from
  // PermissionRequestDispatcher
  virtual void Cancel();

  void NotifyDone();

 private:
  PermissionRequestDispatcher* dispatcher_;

  // The unique ID of this request - used for cancellation from Chromium
  PermissionRequestID request_id_;

  // The frame that initiated this request
  WebFrame* frame_;

  GURL origin_;
  GURL embedder_;

  bool is_cancelled_;
  base::Closure cancel_callback_;

  DISALLOW_COPY_AND_ASSIGN(PermissionRequest);
};

// Implementation of PermissionRequest that allows embedders to respond
// with no parameters. Most types of request will use this class (the main
// exception is media device access permissions, which require responses with
// parameters). If your request requires more information to be exposed, feel
// free to subclass from this 
class SimplePermissionRequest : public PermissionRequest {
 public:
  SimplePermissionRequest(
      const PermissionRequestID& request_id,
      const GURL& origin,
      const GURL& embedder,
      const base::Callback<void(content::PermissionStatus)>& callback);
  ~SimplePermissionRequest() override;

  // Allow the requesting frame access to the desired resource
  void Allow();

  // Deny the requesting frame access to the desired resource
  void Deny();

 private:
  // PermissionRequest implementation
  void Cancel() override;

  // The callback provided by Chromium, which we use to respond to the request
  base::Callback<void(content::PermissionStatus)> callback_;

  DISALLOW_COPY_AND_ASSIGN(SimplePermissionRequest);
};

class MediaAccessPermissionRequest : public PermissionRequest {
 public:
  MediaAccessPermissionRequest(
      WebFrame* frame,
      const GURL& origin,
      const GURL& embedder,
      bool audio_requested,
      bool video_requested,
      const content::MediaResponseCallback& callback);
  ~MediaAccessPermissionRequest() override;

  bool audio_requested() const { return audio_requested_; }

  bool video_requested() const { return video_requested_; }

  // Allow the requesting frame access to the specified resources
  void Allow();

  // Deny the requesting frame access to the specified resources
  void Deny();

 private:
  // PermissionRequest implementation
  void Cancel() override;

  bool audio_requested_;
  bool video_requested_;

  content::MediaResponseCallback callback_;

  DISALLOW_COPY_AND_ASSIGN(MediaAccessPermissionRequest);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_PERMISSION_REQUEST_H_
