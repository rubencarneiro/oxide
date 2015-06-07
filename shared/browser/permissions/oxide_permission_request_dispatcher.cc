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

#include "oxide_permission_request_dispatcher.h"

#include <algorithm>

#include "base/logging.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/media_capture_devices.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "url/gurl.h"

#include "shared/browser/oxide_web_frame.h"

#include "oxide_permission_request.h"
#include "oxide_permission_request_dispatcher_client.h"
#include "oxide_permission_request_id.h"

namespace oxide {

DEFINE_WEB_CONTENTS_USER_DATA_KEY(PermissionRequestDispatcher);

class PermissionRequestDispatcher::IteratorGuard {
 public:
  IteratorGuard(PermissionRequestDispatcher* dispatcher);
  ~IteratorGuard();

 private:
  base::WeakPtr<PermissionRequestDispatcher> dispatcher_;
  bool iterating_original_;

  DISALLOW_COPY_AND_ASSIGN(IteratorGuard);
};

PermissionRequestDispatcher::IteratorGuard::IteratorGuard(
    PermissionRequestDispatcher* dispatcher)
    : dispatcher_(dispatcher->weak_factory_.GetWeakPtr()),
      iterating_original_(dispatcher->iterating_) {
  dispatcher->iterating_ = true;
}

PermissionRequestDispatcher::IteratorGuard::~IteratorGuard() {
  if (!dispatcher_) {
    return;
  }

  dispatcher_->iterating_ = iterating_original_;
  if (!dispatcher_->iterating_) {
    dispatcher_->Compact();
  }
}

PermissionRequestDispatcher::PermissionRequestDispatcher(
    content::WebContents* contents)
    : contents_(contents),
      client_(nullptr),
      iterating_(false),
      weak_factory_(this) {}

void PermissionRequestDispatcher::AddPendingRequest(PermissionRequest* request) {
  DCHECK(!request->dispatcher_);
  DCHECK(std::find(pending_requests_.begin(),
                   pending_requests_.end(),
                   request) ==
         pending_requests_.end());

  request->dispatcher_ = this;
  pending_requests_.push_back(request);
}

void PermissionRequestDispatcher::RemovePendingRequest(
    PermissionRequest* request) {
  DCHECK_EQ(request->dispatcher_, this);
  auto it = std::find(pending_requests_.begin(),
                      pending_requests_.end(),
                      request);
  DCHECK(it != pending_requests_.end());
  if (iterating_) {
    *it = nullptr;
  } else {
    pending_requests_.erase(it);
  }

  request->dispatcher_ = nullptr;
}

void PermissionRequestDispatcher::Compact() {
  DCHECK(!iterating_);

  pending_requests_.erase(
      std::remove(pending_requests_.begin(), pending_requests_.end(), nullptr),
      pending_requests_.end());
}

PermissionRequestDispatcher::~PermissionRequestDispatcher() {
  CancelPendingRequests();
}

void PermissionRequestDispatcher::RequestPermission(
    content::PermissionType permission,
    content::RenderFrameHost* render_frame_host,
    int request_id,
    const GURL& requesting_origin,
    const base::Callback<void(content::PermissionStatus)>& callback) {
  if (!client_) {
    callback.Run(content::PERMISSION_STATUS_DENIED);
    return;
  }

  PermissionRequestID id(
      render_frame_host->GetProcess()->GetID(),
      render_frame_host->GetRoutingID(),
      request_id,
      requesting_origin);

  scoped_ptr<SimplePermissionRequest> request(
      new SimplePermissionRequest(
        id,
        requesting_origin,
        contents_->GetLastCommittedURL().GetOrigin(),
        callback));
  AddPendingRequest(request.get());

  switch (permission) {
    case content::PermissionType::GEOLOCATION:
      client_->RequestGeolocationPermission(request.Pass());
      break;
    default:
      NOTIMPLEMENTED();
      break;
  }
}

void PermissionRequestDispatcher::CancelPermissionRequest(
    content::PermissionType permission,
    content::RenderFrameHost* render_frame_host,
    int request_id,
    const GURL& requesting_origin) {
  PermissionRequestID id(
      render_frame_host->GetProcess()->GetID(),
      render_frame_host->GetRoutingID(),
      request_id,
      requesting_origin);

  IteratorGuard guard(this);
  for (auto request : pending_requests_) {
    if (!request) {
      continue;
    }
    if (request->request_id_ != id) {
      continue;
    }
    request->Cancel();
  }
}

void PermissionRequestDispatcher::RequestMediaAccessPermission(
    content::RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    bool audio,
    bool video,
    const content::MediaResponseCallback& callback) {
  if (!client_) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_PERMISSION_DENIED,
                 nullptr);
    return;
  }

  WebFrame* frame = WebFrame::FromRenderFrameHost(render_frame_host);
  if (!frame) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_PERMISSION_DENIED,
                 nullptr);
    return;
  }

  scoped_ptr<MediaAccessPermissionRequest> req(
      new MediaAccessPermissionRequest(
        frame,
        requesting_origin,
        contents_->GetLastCommittedURL().GetOrigin(),
        audio, video,
        callback));
  AddPendingRequest(req.get());

  client_->RequestMediaAccessPermission(req.Pass());
}

void PermissionRequestDispatcher::CancelPendingRequests() {
  IteratorGuard guard(this);
  for (auto request : pending_requests_) {
    if (!request) {
      continue;
    }
    request->Cancel();
  }
}

void PermissionRequestDispatcher::CancelPendingRequestsForFrame(
    WebFrame* frame) {
  DCHECK(frame);

  IteratorGuard guard(this);
  for (auto request : pending_requests_) {
    if (!request) {
      continue;
    }
    if (request->frame_ != frame) {
      continue;
    }
    request->Cancel();
  }
}

} // namespace oxide
