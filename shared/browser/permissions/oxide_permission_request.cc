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

#include <algorithm>

#include "base/logging.h"

#include "shared/browser/media/oxide_media_capture_devices_dispatcher.h"

#include "oxide_permission_request_dispatcher.h"

namespace oxide {

PermissionRequest::PermissionRequest(const PermissionRequestID& request_id,
                                     WebFrame* frame,
                                     const GURL& origin,
                                     const GURL& embedder)
    : dispatcher_(nullptr),
      request_id_(request_id),
      frame_(frame),
      origin_(origin),
      embedder_(embedder),
      is_cancelled_(false) {}

void PermissionRequest::Cancel() {
  DCHECK(!IsPending());
  DCHECK(!is_cancelled_);

  is_cancelled_ = true;

  if (cancel_callback_.is_null()) {
    return;
  }

  cancel_callback_.Run();
  cancel_callback_.Reset();
}

void PermissionRequest::NotifyDone() {
  dispatcher_->RemovePendingRequest(this);
  DCHECK(!dispatcher_);
}

PermissionRequest::~PermissionRequest() {
  DCHECK(!IsPending());
}

bool PermissionRequest::IsPending() const {
  return !!dispatcher_;
}

void PermissionRequest::SetCancelCallback(const base::Closure& callback) {
  cancel_callback_ = callback;
}

void SimplePermissionRequest::Cancel() {
  Deny();
  PermissionRequest::Cancel();
}

SimplePermissionRequest::SimplePermissionRequest(
    const PermissionRequestID& request_id,
    const GURL& origin,
    const GURL& embedder,
    const base::Callback<void(content::PermissionStatus)>& callback)
    : PermissionRequest(request_id, nullptr, origin, embedder),
      callback_(callback) {}

SimplePermissionRequest::~SimplePermissionRequest() {
  if (IsPending()) {
    Deny();
  }
}

void SimplePermissionRequest::Allow() {
  DCHECK(IsPending());

  callback_.Run(content::PERMISSION_STATUS_GRANTED);
  callback_.Reset();

  NotifyDone();
}

void SimplePermissionRequest::Deny() {
  DCHECK(IsPending());

  callback_.Run(content::PERMISSION_STATUS_DENIED);
  callback_.Reset();

  NotifyDone();
}

void MediaAccessPermissionRequest::Cancel() {
  Deny();
  PermissionRequest::Cancel();
}

MediaAccessPermissionRequest::MediaAccessPermissionRequest(
    WebFrame* frame,
    const GURL& origin,
    const GURL& embedder,
    bool audio_requested,
    bool video_requested,
    const content::MediaResponseCallback& callback)
    : PermissionRequest(PermissionRequestID(),
                        frame,
                        origin,
                        embedder),
      audio_requested_(audio_requested),
      video_requested_(video_requested),
      callback_(callback) {}

MediaAccessPermissionRequest::~MediaAccessPermissionRequest() {
  if (IsPending()) {
    Deny();
  }
}

void MediaAccessPermissionRequest::Allow() {
  DCHECK(IsPending());

  content::MediaStreamDevices devices;
  MediaCaptureDevicesDispatcher::GetInstance()->GetDefaultDevicesForContext(
      nullptr, // XXX(chrisccoulson): Need some way of getting a BrowserContext
      audio_requested_,
      video_requested_,
      &devices);

  callback_.Run(devices,
                devices.empty() ?
                    content::MEDIA_DEVICE_NO_HARDWARE :
                    content::MEDIA_DEVICE_OK,
                nullptr);
  callback_.Reset();

  NotifyDone();
}

void MediaAccessPermissionRequest::Deny() {
  DCHECK(IsPending());

  callback_.Run(content::MediaStreamDevices(),
                content::MEDIA_DEVICE_PERMISSION_DENIED,
                nullptr);
  callback_.Reset();

  NotifyDone();
}

} // namespace oxide
