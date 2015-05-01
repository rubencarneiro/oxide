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

#include <string>
#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "content/public/common/media_stream_request.h"
#include "content/public/common/permission_status.mojom.h"
#include "url/gurl.h"

namespace oxide {

class MediaAccessPermissionRequest;
class PermissionRequest;
class SimplePermissionRequest;

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

 private:
  int render_process_id_;
  int render_view_id_;
  int bridge_id_;
  GURL origin_;
};

// This class tracks PermissionRequests
class PermissionRequestManager {
 public:
  PermissionRequestManager();
  ~PermissionRequestManager();

  // Cancel any pending permission requests
  void CancelPendingRequests();

  // Cancel the pending permission request with the specified |request_id|
  void CancelPendingRequestForID(const PermissionRequestID& request_id);

 private:
  friend class MediaAccessPermissionRequest;
  friend class PermissionRequest;
  friend class SimplePermissionRequest;
  class IteratorGuard;

  // Add a PermissionRequest to this manager
  void AddPendingRequest(PermissionRequest* request);

  // Remove |request| from this manager and invalidate its pointer to this
  void RemovePendingRequest(PermissionRequest* request);

  // Remove empty slots from pending_requests_
  void Compact();

  // Used to indicate that pending_requests_ is being iterated over, and
  // will prevent RemovePendingRequest from mutating it
  bool iterating_;

  typedef std::vector<PermissionRequest*> PermissionRequestVector;

  // This list of PermissionRequests
  std::vector<PermissionRequest*> pending_requests_;

  base::WeakPtrFactory<PermissionRequestManager> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(PermissionRequestManager);
};

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

  // Whether the request has been cancelled by Oxide. A cancelled request can
  // no longer be responded to
  bool is_cancelled() const { return is_cancelled_; }

  // Set a callback to be invoked when this request is cancelled
  void SetCancelCallback(const base::Closure& cancel_callback);

 protected:
  friend class PermissionRequestManager;

  PermissionRequest(PermissionRequestManager* manager,
                    const PermissionRequestID& request_id,
                    const GURL& origin,
                    const GURL& embedder);

  // Cancel this request and run the cancel callback. This is only called from
  // PermissionRequestManager
  virtual void Cancel();

  PermissionRequestManager* manager_;

 private:
  PermissionRequestID request_id_;

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
      PermissionRequestManager* manager,
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
      PermissionRequestManager* manager,
      const PermissionRequestID& request_id,
      const GURL& origin,
      const GURL& embedder,
      bool audio_requested,
      bool video_requested,
      const content::MediaResponseCallback& callback);
  ~MediaAccessPermissionRequest() override;

  bool audio_requested() const { return audio_requested_; }

  bool video_requested() const { return video_requested_; }

  void Allow();
  void Allow(const std::string& audio_device_id,
             const std::string& video_device_id);

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
