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
#include <utility>

#include "base/logging.h"
#include "content/public/browser/navigation_details.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "url/gurl.h"

#include "shared/browser/oxide_render_object_id.h"
#include "shared/browser/oxide_web_frame.h"

#include "oxide_permission_request.h"
#include "oxide_permission_request_dispatcher_client.h"

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
    : content::WebContentsObserver(contents),
      client_(nullptr),
      iterating_(false),
      next_request_id_(0),
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

void PermissionRequestDispatcher::CancelPendingRequests() {
  IteratorGuard guard(this);
  for (auto request : pending_requests_) {
    if (!request) {
      continue;
    }
    request->Cancel(true);
  }
}

void PermissionRequestDispatcher::CancelPendingRequestsForFrame(
    content::RenderFrameHost* frame) {
  RenderFrameHostID frame_id = RenderFrameHostID(frame);

  IteratorGuard guard(this);
  for (auto request : pending_requests_) {
    if (!request) {
      continue;
    }
    if (!request->frame_id_.IsValid()) {
      continue;
    }
    if (request->frame_id_ != frame_id) {
      continue;
    }
    request->Cancel(true);
  }
}

void PermissionRequestDispatcher::RenderFrameDeleted(
    content::RenderFrameHost* render_frame_host) {
  CancelPendingRequestsForFrame(render_frame_host);
}

void PermissionRequestDispatcher::RenderFrameHostChanged(
    content::RenderFrameHost* old_host,
    content::RenderFrameHost* new_host) {
  if (!old_host) {
    return;
  }

  CancelPendingRequestsForFrame(old_host);
}

void PermissionRequestDispatcher::DidNavigateAnyFrame(
    content::RenderFrameHost* render_frame_host,
    const content::LoadCommittedDetails& details,
    const content::FrameNavigateParams& params) {
  if (details.is_in_page) {
    return;
  }

  CancelPendingRequestsForFrame(render_frame_host);
}

PermissionRequestDispatcher::~PermissionRequestDispatcher() {
  CancelPendingRequests();
}

// static
PermissionRequestDispatcher* PermissionRequestDispatcher::FromWebContents(
    content::WebContents* contents) {
  return content::WebContentsUserData<PermissionRequestDispatcher>
      ::FromWebContents(contents);
}

bool PermissionRequestDispatcher::CanDispatchRequest() const {
  return !!client_;
}

int PermissionRequestDispatcher::RequestPermission(
    content::PermissionType permission,
    content::RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    const PermissionRequestCallback& callback) {
  if (!client_) {
    callback.Run(PERMISSION_REQUEST_RESPONSE_CANCEL);
    return -1;
  }

  int request_id = next_request_id_++;

  scoped_ptr<PermissionRequest> request(
      new PermissionRequest(
        request_id,
        RenderFrameHostID(),
        requesting_origin,
        web_contents()->GetLastCommittedURL().GetOrigin(),
        callback));
  AddPendingRequest(request.get());

  switch (permission) {
    case content::PermissionType::GEOLOCATION:
      client_->RequestGeolocationPermission(std::move(request));
      break;
    case content::PermissionType::NOTIFICATIONS:
      client_->RequestNotificationPermission(std::move(request));
      break;
    default:
      NOTIMPLEMENTED();
      break;
  }

  return request_id;
}

void PermissionRequestDispatcher::CancelPermissionRequest(int request_id) {
  IteratorGuard guard(this);
  for (auto request : pending_requests_) {
    if (!request) {
      continue;
    }
    if (request->request_id_ != request_id) {
      continue;
    }
    request->Cancel(true);
  }
}

int PermissionRequestDispatcher::RequestMediaAccessPermission(
    content::RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    bool audio,
    bool video,
    const PermissionRequestCallback& callback) {
  if (!client_) {
    callback.Run(PERMISSION_REQUEST_RESPONSE_CANCEL);
    return -1;
  }

  int request_id = next_request_id_++;

  scoped_ptr<MediaAccessPermissionRequest> req(
      new MediaAccessPermissionRequest(
        request_id,
        RenderFrameHostID(render_frame_host),
        requesting_origin,
        web_contents()->GetLastCommittedURL().GetOrigin(),
        audio, video,
        callback));
  AddPendingRequest(req.get());

  client_->RequestMediaAccessPermission(std::move(req));

  return request_id;
}

} // namespace oxide
