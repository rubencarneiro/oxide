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

PermissionRequestID::PermissionRequestID(int render_process_id,
                                         int render_view_id,
                                         int bridge_id,
                                         const GURL& origin)
    : render_process_id_(render_process_id),
      render_view_id_(render_view_id),
      bridge_id_(bridge_id),
      origin_(origin) {}

PermissionRequestID::PermissionRequestID()
    : render_process_id_(-1),
      render_view_id_(-1),
      bridge_id_(-1) {}

PermissionRequestID::~PermissionRequestID() {}

bool PermissionRequestID::IsValid() const {
  return render_process_id_ > -1 && render_view_id_ > -1 && bridge_id_ > -1;
}

bool PermissionRequestID::operator==(const PermissionRequestID& other) const {
  return render_process_id_ == other.render_process_id_ &&
         render_view_id_ == other.render_view_id_ &&
         bridge_id_ == other.bridge_id_ &&
         origin_ == other.origin_;
}

class PermissionRequestManager::IteratorGuard {
 public:
  IteratorGuard(PermissionRequestManager* manager);
  ~IteratorGuard();

 private:
  base::WeakPtr<PermissionRequestManager> manager_;
  bool iterating_original_;

  DISALLOW_COPY_AND_ASSIGN(IteratorGuard);
};

PermissionRequestManager::IteratorGuard::IteratorGuard(
    PermissionRequestManager* manager)
    : manager_(manager->weak_factory_.GetWeakPtr()),
      iterating_original_(manager->iterating_) {
  manager->iterating_ = true;
}

PermissionRequestManager::IteratorGuard::~IteratorGuard() {
  if (!manager_) {
    return;
  }

  manager_->iterating_ = iterating_original_;
  if (!manager_->iterating_) {
    manager_->Compact();
  }
}

void PermissionRequestManager::AddPendingRequest(PermissionRequest* request) {
  DCHECK_EQ(request->manager_, this);
  DCHECK(std::find(
      pending_requests_.begin(),
      pending_requests_.end(),
      request) == pending_requests_.end());

  pending_requests_.push_back(request);
}

void PermissionRequestManager::RemovePendingRequest(
    PermissionRequest* request) {
  DCHECK_EQ(request->manager_, this);
  auto it =
      std::find(pending_requests_.begin(), pending_requests_.end(), request);
  DCHECK(it != pending_requests_.end());
  if (iterating_) {
    *it = nullptr;
  } else {
    pending_requests_.erase(it);
  }

  request->manager_ = nullptr;
}

void PermissionRequestManager::Compact() {
  DCHECK(!iterating_);

  pending_requests_.erase(
      std::remove(pending_requests_.begin(), pending_requests_.end(), nullptr),
      pending_requests_.end());
}

PermissionRequestManager::PermissionRequestManager()
    : iterating_(false),
      weak_factory_(this) {}

PermissionRequestManager::~PermissionRequestManager() {
  CancelPendingRequests();
}

void PermissionRequestManager::CancelPendingRequests() {
  IteratorGuard guard(this);
  for (auto it = pending_requests_.begin();
       it != pending_requests_.end(); ++it) {
    PermissionRequest* request = *it;
    if (!request) {
      continue;
    }

    RemovePendingRequest(request);
    request->Cancel();
  }
}

void PermissionRequestManager::CancelPendingRequestForID(
    const PermissionRequestID& request_id) {
  DCHECK(request_id.IsValid());

  for (auto it = pending_requests_.begin();
       it != pending_requests_.end(); ++it) {
    PermissionRequest* request = *it;
    if (request->request_id_ == request_id) {
      RemovePendingRequest(request);
      request->Cancel();
      break;
    }
  }
}

void PermissionRequestManager::CancelPendingRequestsForFrame(WebFrame* frame) {
  DCHECK(frame);

  IteratorGuard guard(this);
  for (auto it = pending_requests_.begin();
       it != pending_requests_.end(); ++it) {
    PermissionRequest* request = *it;
    if (request->frame_ != frame) {
      continue;
    }

    RemovePendingRequest(request);
    request->Cancel();
  }
}

PermissionRequest::PermissionRequest(PermissionRequestManager* manager,
                                     const PermissionRequestID& request_id,
                                     WebFrame* frame,
                                     const GURL& origin,
                                     const GURL& embedder)
    : manager_(manager),
      request_id_(request_id),
      frame_(frame),
      origin_(origin),
      embedder_(embedder),
      is_cancelled_(false) {
  DCHECK(manager_);
  manager_->AddPendingRequest(this);
}

void PermissionRequest::Cancel() {
  DCHECK(!is_cancelled_);

  is_cancelled_ = true;

  if (!cancel_callback_.is_null()) {
    cancel_callback_.Run();
    cancel_callback_.Reset();
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

void SimplePermissionRequest::Cancel() {
  DCHECK(!callback_.is_null());

  callback_.Run(content::PERMISSION_STATUS_DENIED);
  callback_.Reset();

  PermissionRequest::Cancel();
}

SimplePermissionRequest::SimplePermissionRequest(
    PermissionRequestManager* manager,
    const PermissionRequestID& request_id,
    const GURL& origin,
    const GURL& embedder,
    const base::Callback<void(content::PermissionStatus)>& callback)
    : PermissionRequest(manager, request_id, nullptr, origin, embedder),
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

  manager_->RemovePendingRequest(this);
}

void SimplePermissionRequest::Deny() {
  DCHECK(!callback_.is_null());

  callback_.Run(content::PERMISSION_STATUS_DENIED);
  callback_.Reset();

  manager_->RemovePendingRequest(this);
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
    PermissionRequestManager* manager,
    WebFrame* frame,
    const GURL& origin,
    const GURL& embedder,
    bool audio_requested,
    bool video_requested,
    const content::MediaResponseCallback& callback)
    : PermissionRequest(manager,
                        PermissionRequestID(),
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
    } else {
      LOG(WARNING)
          << "No audio capture device found. This might happen if it was "
          << "recently removed from the system";
    }
  }

  if (video_requested_) {
    const content::MediaStreamDevice* device =
        GetRequestedOrDefaultDevice(
          content::MediaCaptureDevices::GetInstance()->GetVideoCaptureDevices(),
          video_device_id);
    if (device) {
      devices.push_back(*device);
    } else {
      LOG(WARNING)
          << "No video capture device found. This might happen if it was "
          << "recently removed from the system";
    }
  }

  callback_.Run(devices,
                devices.empty() ?
                    content::MEDIA_DEVICE_NO_HARDWARE :
                    content::MEDIA_DEVICE_OK,
                nullptr);
  callback_.Reset();

  manager_->RemovePendingRequest(this);
}

void MediaAccessPermissionRequest::Deny() {
  DCHECK(!callback_.is_null());

  callback_.Run(content::MediaStreamDevices(),
                content::MEDIA_DEVICE_PERMISSION_DENIED,
                nullptr);
  callback_.Reset();

  manager_->RemovePendingRequest(this);
}

} // namespace oxide
