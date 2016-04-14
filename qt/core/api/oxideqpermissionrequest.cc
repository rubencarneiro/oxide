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

#include "oxideqpermissionrequest.h"
#include "oxideqpermissionrequest_p.h"

#include <utility>

#include <QString>
#include <QtDebug>
#include <string>

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "url/gurl.h"

#include "shared/browser/permissions/oxide_permission_request.h"

#include "oxideqglobal_p.h"

void OxideQPermissionRequestPrivate::OnCancelled() {
  Q_Q(OxideQPermissionRequest);

  Q_EMIT q->cancelled();
}

OxideQPermissionRequestPrivate::OxideQPermissionRequestPrivate(
    scoped_ptr<oxide::PermissionRequest> request)
    : q_ptr(nullptr),
      request_(std::move(request)) {}

bool OxideQPermissionRequestPrivate::canRespond() const {
  if (request_->is_cancelled()) {
    qWarning() <<
        "OxideQPermissionRequest: Can't respond to a cancelled "
        "permission request";
    return false;
  }

  if (!request_->IsPending()) {
    qWarning() <<
        "OxideQPermissionRequest: Can only respond once to a permission "
        "request";
    return false;
  }

  return true;
}

OxideQPermissionRequestPrivate::~OxideQPermissionRequestPrivate() {}

// static
OxideQPermissionRequest* OxideQPermissionRequestPrivate::Create(
    scoped_ptr<oxide::PermissionRequest> request) {
  DCHECK(request);

  return new OxideQPermissionRequest(
      *new OxideQPermissionRequestPrivate(std::move(request)));
}

OxideQGeolocationPermissionRequestPrivate::
    OxideQGeolocationPermissionRequestPrivate(
      scoped_ptr<oxide::PermissionRequest> request)
      : OxideQPermissionRequestPrivate(std::move(request)) {}

OxideQGeolocationPermissionRequestPrivate::
    ~OxideQGeolocationPermissionRequestPrivate() {}

// static
OxideQGeolocationPermissionRequest*
OxideQGeolocationPermissionRequestPrivate::Create(
    scoped_ptr<oxide::PermissionRequest> request) {
  DCHECK(request);

  return new OxideQGeolocationPermissionRequest(
      *new OxideQGeolocationPermissionRequestPrivate(std::move(request)));
}

OxideQMediaAccessPermissionRequestPrivate::
    OxideQMediaAccessPermissionRequestPrivate(
      scoped_ptr<oxide::MediaAccessPermissionRequest> request)
      : OxideQPermissionRequestPrivate(std::move(request)) {}

oxide::MediaAccessPermissionRequest*
OxideQMediaAccessPermissionRequestPrivate::request() const {
  return static_cast<oxide::MediaAccessPermissionRequest*>(request_.get());
}

OxideQMediaAccessPermissionRequestPrivate::
    ~OxideQMediaAccessPermissionRequestPrivate() {}

// static
OxideQMediaAccessPermissionRequest*
OxideQMediaAccessPermissionRequestPrivate::Create(
    scoped_ptr<oxide::MediaAccessPermissionRequest> request) {
  DCHECK(request);

  return new OxideQMediaAccessPermissionRequest(
      *new OxideQMediaAccessPermissionRequestPrivate(std::move(request)));
}

OxideQPermissionRequest::OxideQPermissionRequest(
    OxideQPermissionRequestPrivate& dd)
    : d_ptr(&dd) {
  Q_D(OxideQPermissionRequest);

  d->q_ptr = this;
  d->request_->SetCancelCallback(
      base::Bind(&OxideQPermissionRequestPrivate::OnCancelled,
                 // The callback cannot run after |d| is deleted, as it
                 // exclusively owns |request_|
                 base::Unretained(d)));
}

OxideQPermissionRequest::~OxideQPermissionRequest() {}

QUrl OxideQPermissionRequest::origin() const {
  Q_D(const OxideQPermissionRequest);

  return QUrl(QString::fromStdString(d->request_->origin().spec()));;
}

QUrl OxideQPermissionRequest::embedder() const {
  Q_D(const OxideQPermissionRequest);

  return QUrl(QString::fromStdString(d->request_->embedder().spec()));
}

QUrl OxideQPermissionRequest::url() const {
  WARN_DEPRECATED_API_USAGE() <<
      "OxideQPermissionRequest::url is deprecated. Please use "
      "OxideQPermissionRequest::origin instead";

  return origin();
}

bool OxideQPermissionRequest::isCancelled() const {
  Q_D(const OxideQPermissionRequest);

  return d->request_->is_cancelled();
}

void OxideQPermissionRequest::allow() {
  Q_D(OxideQPermissionRequest);

  if (!d->canRespond()) {
    return;
  }

  d->request_->Allow();
}

void OxideQPermissionRequest::deny() {
  Q_D(OxideQPermissionRequest);

  if (!d->canRespond()) {
    return;
  }

  d->request_->Deny();
}

OxideQGeolocationPermissionRequest::OxideQGeolocationPermissionRequest(
    OxideQGeolocationPermissionRequestPrivate& dd)
    : OxideQPermissionRequest(dd) {}

OxideQGeolocationPermissionRequest::~OxideQGeolocationPermissionRequest() {}

void OxideQGeolocationPermissionRequest::accept() {
  WARN_DEPRECATED_API_USAGE() <<
      "OxideQGeolocationPermissionRequest::accept is deprecated. Please use "
      "OxideQPermissionRequest::allow instead";

  allow();
}

OxideQMediaAccessPermissionRequest::OxideQMediaAccessPermissionRequest(
    OxideQMediaAccessPermissionRequestPrivate& dd)
    : OxideQPermissionRequest(dd) {}

OxideQMediaAccessPermissionRequest::~OxideQMediaAccessPermissionRequest() {}

bool OxideQMediaAccessPermissionRequest::isForAudio() const {
  Q_D(const OxideQMediaAccessPermissionRequest);

  return d->request()->audio_requested();
}

bool OxideQMediaAccessPermissionRequest::isForVideo() const {
  Q_D(const OxideQMediaAccessPermissionRequest);

  return d->request()->video_requested();
}

QString OxideQMediaAccessPermissionRequest::requestedAudioDeviceId() const {
  Q_D(const OxideQMediaAccessPermissionRequest);

  return QString::fromStdString(d->request()->requested_audio_device_id());
}

QString OxideQMediaAccessPermissionRequest::requestedVideoDeviceId() const {
  Q_D(const OxideQMediaAccessPermissionRequest);

  return QString::fromStdString(d->request()->requested_video_device_id());
}
