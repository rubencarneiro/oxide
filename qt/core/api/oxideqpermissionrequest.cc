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

#include <QString>
#include <QtDebug>
#include <string>

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "url/gurl.h"

#include "shared/browser/permissions/oxide_permission_request.h"

void OxideQPermissionRequestPrivate::OnCancelled() {
  Q_Q(OxideQPermissionRequest);

  Q_EMIT q->cancelled();
}

OxideQPermissionRequestPrivate::OxideQPermissionRequestPrivate(
    scoped_ptr<oxide::PermissionRequest> request)
    : q_ptr(nullptr),
      request_(request.Pass()) {}

OxideQPermissionRequestPrivate::~OxideQPermissionRequestPrivate() {}

OxideQSimplePermissionRequestPrivate::OxideQSimplePermissionRequestPrivate(
    scoped_ptr<oxide::SimplePermissionRequest> request)
    : OxideQPermissionRequestPrivate(request.Pass()) {}

bool OxideQSimplePermissionRequestPrivate::canRespond() const {
  if (request_->is_cancelled()) {
    qWarning() <<
        "OxideQSimplePermissionRequest: Can't respond to a cancelled "
        "permission request";
    return false;
  }

  if (!request_->IsPending()) {
    qWarning() <<
        "OxideQSimplePermissionRequest: Can only respond once to a permission "
        "request";
    return false;
  }

  return true;
}

oxide::SimplePermissionRequest*
OxideQSimplePermissionRequestPrivate::request() const {
  return static_cast<oxide::SimplePermissionRequest *>(request_.get());
}

OxideQSimplePermissionRequestPrivate::
    ~OxideQSimplePermissionRequestPrivate() {}

// static
OxideQSimplePermissionRequest* OxideQSimplePermissionRequestPrivate::Create(
    scoped_ptr<oxide::SimplePermissionRequest> request) {
  DCHECK(request);

  return new OxideQSimplePermissionRequest(
      *new OxideQSimplePermissionRequestPrivate(request.Pass()));
}

OxideQGeolocationPermissionRequestPrivate::
    OxideQGeolocationPermissionRequestPrivate(
      scoped_ptr<oxide::SimplePermissionRequest> request)
      : OxideQSimplePermissionRequestPrivate(request.Pass()) {}

OxideQGeolocationPermissionRequestPrivate::
    ~OxideQGeolocationPermissionRequestPrivate() {}

// static
OxideQGeolocationPermissionRequest*
OxideQGeolocationPermissionRequestPrivate::Create(
    scoped_ptr<oxide::SimplePermissionRequest> request) {
  DCHECK(request);

  return new OxideQGeolocationPermissionRequest(
      *new OxideQGeolocationPermissionRequestPrivate(request.Pass()));
}

OxideQMediaAccessPermissionRequestPrivate::
    OxideQMediaAccessPermissionRequestPrivate(
      scoped_ptr<oxide::MediaAccessPermissionRequest> request)
      : OxideQPermissionRequestPrivate(request.Pass()) {}

bool OxideQMediaAccessPermissionRequestPrivate::canRespond() const {
  if (request_->is_cancelled()) {
    qWarning() <<
        "OxideQMediaAccessPermissionRequest: Can't respond to a cancelled "
        "permission request";
    return false;
  }

  if (!request_->IsPending()) {
    qWarning() <<
        "OxideQMediaAccessPermissionRequest: Can only respond once to a "
        "permission request";
    return false;
  }

  return true;
}

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
      *new OxideQMediaAccessPermissionRequestPrivate(request.Pass()));
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
  static bool warn_once = false;
  if (!warn_once) {
    warn_once = true;
    qWarning() <<
        "OxideQPermissionRequest::url is deprecated. Please use "
        "OxideQPermissionRequest::origin instead";
  }

  return origin();
}

bool OxideQPermissionRequest::isCancelled() const {
  Q_D(const OxideQPermissionRequest);

  return d->request_->is_cancelled();
}

OxideQSimplePermissionRequest::OxideQSimplePermissionRequest(
    OxideQSimplePermissionRequestPrivate& dd)
    : OxideQPermissionRequest(dd) {}

OxideQSimplePermissionRequest::~OxideQSimplePermissionRequest() {}

void OxideQSimplePermissionRequest::allow() {
  Q_D(OxideQSimplePermissionRequest);

  if (!d->canRespond()) {
    return;
  }

  d->request()->Allow();
}

void OxideQSimplePermissionRequest::deny() {
  Q_D(OxideQSimplePermissionRequest);

  if (!d->canRespond()) {
    return;
  }

  d->request()->Deny();
}

OxideQGeolocationPermissionRequest::OxideQGeolocationPermissionRequest(
    OxideQGeolocationPermissionRequestPrivate& dd)
    : OxideQSimplePermissionRequest(dd) {}

OxideQGeolocationPermissionRequest::~OxideQGeolocationPermissionRequest() {}

QUrl OxideQGeolocationPermissionRequest::origin() const {
  return OxideQPermissionRequest::origin();
}

void OxideQGeolocationPermissionRequest::accept() {
  static bool warn_once = false;
  if (!warn_once) {
    warn_once = true;
    qWarning() <<
        "OxideQGeolocationPermissionRequest::accept is deprecated. Please use "
        "OxideQPermissionRequest::allow instead";
  }

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

void OxideQMediaAccessPermissionRequest::allow() {
  Q_D(OxideQMediaAccessPermissionRequest);

  if (!d->canRespond()) {
    return;
  }

  d->request()->Allow();
}

void OxideQMediaAccessPermissionRequest::deny() {
  Q_D(OxideQMediaAccessPermissionRequest);

  if (!d->canRespond()) {
    return;
  }

  d->request()->Deny();
}
