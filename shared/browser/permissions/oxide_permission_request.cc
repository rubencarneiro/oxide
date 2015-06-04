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
#include "content/public/browser/media_capture_devices.h"

#include "oxide_permission_request_dispatcher.h"

namespace oxide {

namespace {

const content::MediaStreamDevice* GetRequestedOrDefaultDevice(
    const content::MediaStreamDevices& devices,
    const std::string& device_id) {
  if (devices.size() == 0) {
    return nullptr;
  }

  if (!device_id.empty()) {
    for (auto it = devices.begin(); it != devices.end(); ++it) {
      const content::MediaStreamDevice& device = *it;
      if (device.id == device_id) {
        return &device;
      }
    }
  }

  return &devices[0];
}

}

PermissionRequest::PermissionRequest(const PermissionRequestID& request_id,
                                     WebFrame* frame,
                                     const GURL& origin,
                                     const GURL& embedder)
    : dispatcher_(nullptr),
      request_id_(request_id),
      frame_(frame),
      origin_(origin),
      embedder_(embedder) {}

void PermissionRequest::Cancel() {
  if (cancel_callback_.is_null()) {
    return;
  }

  cancel_callback_.Run();
  cancel_callback_.Reset();
}

void PermissionRequest::NotifyDone() {
  if (!dispatcher_) {
    return;
  }

  dispatcher_->RemovePendingRequest(this);
}

PermissionRequest::~PermissionRequest() {
  if (dispatcher_) {
    dispatcher_->RemovePendingRequest(this);
  }
}

void PermissionRequest::SetCancelCallback(const base::Closure& callback) {
  DCHECK(dispatcher_);
  cancel_callback_ = callback;
}

void SimplePermissionRequest::Cancel() {
  DCHECK(!callback_.is_null());

  callback_.Run(content::PERMISSION_STATUS_DENIED);
  callback_.Reset();

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
  if (!callback_.is_null()) {
    Deny();
  }
}

void SimplePermissionRequest::Allow() {
  DCHECK(!callback_.is_null());

  callback_.Run(content::PERMISSION_STATUS_GRANTED);
  callback_.Reset();

  NotifyDone();
}

void SimplePermissionRequest::Deny() {
  DCHECK(!callback_.is_null());

  callback_.Run(content::PERMISSION_STATUS_DENIED);
  callback_.Reset();

  NotifyDone();
}

void MediaAccessPermissionRequest::Cancel() {
  DCHECK(!callback_.is_null());

  callback_.Run(content::MediaStreamDevices(),
                content::MEDIA_DEVICE_PERMISSION_DENIED,
                nullptr);
  callback_.Reset();

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
  if (!callback_.is_null()) {
    Deny();
  }
}

void MediaAccessPermissionRequest::Allow() {
  Allow(std::string(), std::string());
}

void MediaAccessPermissionRequest::Allow(const std::string& audio_device_id,
                                         const std::string& video_device_id) {
  DCHECK(!callback_.is_null());

  content::MediaStreamDevices devices;

  if (audio_requested_) {
    const content::MediaStreamDevice* device =
        GetRequestedOrDefaultDevice(
          content::MediaCaptureDevices::GetInstance()->GetAudioCaptureDevices(),
          audio_device_id);
    if (device) {
      devices.push_back(*device);
    }
  }

  if (video_requested_) {
    const content::MediaStreamDevice* device =
        GetRequestedOrDefaultDevice(
          content::MediaCaptureDevices::GetInstance()->GetVideoCaptureDevices(),
          video_device_id);
    if (device) {
      devices.push_back(*device);
    }
  }

  callback_.Run(devices,
                devices.empty() ?
                    content::MEDIA_DEVICE_NO_HARDWARE :
                    content::MEDIA_DEVICE_OK,
                nullptr);
  callback_.Reset();

  NotifyDone();
}

void MediaAccessPermissionRequest::Deny() {
  DCHECK(!callback_.is_null());

  callback_.Run(content::MediaStreamDevices(),
                content::MEDIA_DEVICE_PERMISSION_DENIED,
                nullptr);
  callback_.Reset();

  NotifyDone();
}

} // namespace oxide
