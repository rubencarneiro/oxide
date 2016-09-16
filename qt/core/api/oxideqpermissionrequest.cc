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
    std::unique_ptr<oxide::PermissionRequest> request)
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
    std::unique_ptr<oxide::PermissionRequest> request) {
  DCHECK(request);

  return new OxideQPermissionRequest(
      *new OxideQPermissionRequestPrivate(std::move(request)));
}

OxideQGeolocationPermissionRequestPrivate::
    OxideQGeolocationPermissionRequestPrivate(
      std::unique_ptr<oxide::PermissionRequest> request)
      : OxideQPermissionRequestPrivate(std::move(request)) {}

OxideQGeolocationPermissionRequestPrivate::
    ~OxideQGeolocationPermissionRequestPrivate() {}

// static
OxideQGeolocationPermissionRequest*
OxideQGeolocationPermissionRequestPrivate::Create(
    std::unique_ptr<oxide::PermissionRequest> request) {
  DCHECK(request);

  return new OxideQGeolocationPermissionRequest(
      *new OxideQGeolocationPermissionRequestPrivate(std::move(request)));
}

OxideQMediaAccessPermissionRequestPrivate::
    OxideQMediaAccessPermissionRequestPrivate(
      std::unique_ptr<oxide::MediaAccessPermissionRequest> request)
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
    std::unique_ptr<oxide::MediaAccessPermissionRequest> request) {
  DCHECK(request);

  return new OxideQMediaAccessPermissionRequest(
      *new OxideQMediaAccessPermissionRequestPrivate(std::move(request)));
}

/*!
\class OxideQPermissionRequest
\inmodule OxideQtCore
\inheaderfile oxideqpermissionrequest.h

\brief Generic permission request

OxideQPermissionRequest represents a request for permission to access a specific
resource. The permission request does not indicate the type of resource that the
request is for, but instead, this is indicated by the source of the request.

It is assumed that the application will display a UI to request permission from
the user.

The origin of the site that this request originates from can be accessed using
\l{origin}.

The application can respond to this request by calling \l{allow} (which will
permit access to the resource) or \l{deny} (which will deny access to the
resource).

If the request instance is deleted before the application has called \l{allow}
or \l{deny}, the request will be automatically declined.
*/

/*!
\internal
*/

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

/*!
Destroy this permission request. If the application has not yet responded by
calling \l{allow} or \l{deny}, access to the resource will be automatically
declined.
*/

OxideQPermissionRequest::~OxideQPermissionRequest() {}

/*!
\property OxideQPermissionRequest::origin

The origin of the page requesting access to the resource.
*/

QUrl OxideQPermissionRequest::origin() const {
  Q_D(const OxideQPermissionRequest);

  return QUrl(QString::fromStdString(d->request_->origin().spec()));;
}

/*!
\property OxideQPermissionRequest::embedder

The origin of the top-level page that hosts the page requesting access to the
resource.

If the request originates from the main frame, this will be equal to
\l{origin}.
*/

QUrl OxideQPermissionRequest::embedder() const {
  Q_D(const OxideQPermissionRequest);

  return QUrl(QString::fromStdString(d->request_->embedder().spec()));
}

/*!
\property OxideQPermissionRequest::url
\deprecated
*/

QUrl OxideQPermissionRequest::url() const {
  WARN_DEPRECATED_API_USAGE() <<
      "OxideQPermissionRequest::url is deprecated. Please use "
      "OxideQPermissionRequest::origin instead";

  return origin();
}

/*!
\property OxideQPermissionRequest::isCancelled

The permission request has been cancelled. This could be because the originating
frame navigated to another page or was deleted.

If the application is displaying a permission request UI to the user, it should
hide it when this property indicates that the request has been cancelled.
*/

bool OxideQPermissionRequest::isCancelled() const {
  Q_D(const OxideQPermissionRequest);

  return d->request_->is_cancelled();
}

/*!
Permit access to the resource for which this permission request requests access,
for the specified \l{origin} / \l{embedder} combination.
*/

void OxideQPermissionRequest::allow() {
  Q_D(OxideQPermissionRequest);

  if (!d->canRespond()) {
    return;
  }

  d->request_->Allow();
}

/*!
Decline access to the resource for which this permission request requests
access.
*/

void OxideQPermissionRequest::deny() {
  Q_D(OxideQPermissionRequest);

  if (!d->canRespond()) {
    return;
  }

  d->request_->Deny();
}

/*!
\class OxideQGeolocationPermissionRequest
\inmodule OxideQtCore
\inheaderfile oxideqpermissionrequest.h

\brief Geolocation permission request

OxideQGeolocationPermissionRequest represents a request for permission to access
the current location. This subclass exists purely for legacy purposes.

Please refer to the documentation for OxideQPermissionRequest.
*/

OxideQGeolocationPermissionRequest::OxideQGeolocationPermissionRequest(
    OxideQGeolocationPermissionRequestPrivate& dd)
    : OxideQPermissionRequest(dd) {}

OxideQGeolocationPermissionRequest::~OxideQGeolocationPermissionRequest() {}

/*!
\deprecated
*/

void OxideQGeolocationPermissionRequest::accept() {
  WARN_DEPRECATED_API_USAGE() <<
      "OxideQGeolocationPermissionRequest::accept is deprecated. Please use "
      "OxideQPermissionRequest::allow instead";

  allow();
}

/*!
\class OxideQMediaAccessPermissionRequest
\inmodule OxideQtCore
\inheaderfile oxideqpermissionrequest.h

\brief Media-device access permission request

OxideQMediaAccessPermissionRequest represents a request for permission to
access media capture devices, via \e{MediaDevices.getUserMedia()}.

Applications can use isForAudio and isForVideo to determine whether this is a
request to access audio and/or video capture devices.

This is a subclass of OxideQPermissionRequest. Please see the documentation for
OxideQPermissionRequest for details of the inherited functionality.
*/

OxideQMediaAccessPermissionRequest::OxideQMediaAccessPermissionRequest(
    OxideQMediaAccessPermissionRequestPrivate& dd)
    : OxideQPermissionRequest(dd) {}

OxideQMediaAccessPermissionRequest::~OxideQMediaAccessPermissionRequest() {}

/*!
\property OxideQMediaAccessPermissionRequest::isForAudio

Whether this is a request to access audio capture devices.
*/

bool OxideQMediaAccessPermissionRequest::isForAudio() const {
  Q_D(const OxideQMediaAccessPermissionRequest);

  return d->request()->audio_requested();
}

/*!
\property OxideQMediaAccessPermissionRequest::isForVideo

Whether this is a request to access video capture devices.
*/

bool OxideQMediaAccessPermissionRequest::isForVideo() const {
  Q_D(const OxideQMediaAccessPermissionRequest);

  return d->request()->video_requested();
}
