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

#include "oxide_permission_request.h"

#include "base/logging.h"

#include "oxide_permission_request_dispatcher.h"

namespace oxide {

void PermissionRequest::Cancel(bool run_callback) {
  DCHECK(!is_cancelled_);
  is_cancelled_ = true;

  Respond(PERMISSION_REQUEST_RESPONSE_CANCEL);

  if (cancel_callback_.is_null()) {
    return;
  }

  if (run_callback) {
    cancel_callback_.Run();
  }  
  cancel_callback_.Reset();
}

void PermissionRequest::Respond(PermissionRequestResponse response) {
  DCHECK(IsPending());

  callback_.Run(response);
  callback_.Reset();

  dispatcher_->RemovePendingRequest(this);
  DCHECK(!dispatcher_);
}

PermissionRequest::PermissionRequest(
    int request_id,
    const RenderFrameHostID& frame_id,
    const GURL& origin,
    const GURL& embedder,
    const base::Callback<void(PermissionRequestResponse)>& callback)
    : request_id_(request_id),
      frame_id_(frame_id),
      dispatcher_(nullptr),
      origin_(origin),
      embedder_(embedder),
      is_cancelled_(false),
      callback_(callback) {}

PermissionRequest::~PermissionRequest() {
  if (IsPending()) {
    Cancel(false);
  }
}

bool PermissionRequest::IsPending() const {
  return !!dispatcher_;
}

void PermissionRequest::SetCancelCallback(const base::Closure& callback) {
  cancel_callback_ = callback;
}

void PermissionRequest::Allow() {
  Respond(PERMISSION_REQUEST_RESPONSE_ALLOW);
}

void PermissionRequest::Deny() {
  Respond(PERMISSION_REQUEST_RESPONSE_DENY);
}

MediaAccessPermissionRequest::MediaAccessPermissionRequest(
    int request_id,
    const RenderFrameHostID& frame_id,
    const GURL& origin,
    const GURL& embedder,
    bool audio_requested,
    bool video_requested,
    const std::string& requested_audio_device_id,
    const std::string& requested_video_device_id,
    const base::Callback<void(PermissionRequestResponse)>& callback)
    : PermissionRequest(request_id,
                        frame_id,
                        origin,
                        embedder,
                        callback),
      audio_requested_(audio_requested),
      video_requested_(video_requested),
      requested_audio_device_id_(requested_audio_device_id),
      requested_video_device_id_(requested_video_device_id) {}

MediaAccessPermissionRequest::~MediaAccessPermissionRequest() {}

} // namespace oxide
