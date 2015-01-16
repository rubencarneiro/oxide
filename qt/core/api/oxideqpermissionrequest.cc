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

#include "oxideqpermissionrequest.h"
#include "oxideqpermissionrequest_p.h"

#include <QString>
#include <QtDebug>
#include <string>

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "url/gurl.h"

#include "shared/browser/oxide_permission_request.h"

void OxideQPermissionRequestPrivate::OnCancelled() {
  Q_Q(OxideQPermissionRequest);

  DCHECK(!is_cancelled_);
  is_cancelled_ = true;

  Q_EMIT q->cancelled();
}

OxideQPermissionRequestPrivate::OxideQPermissionRequestPrivate(
    const QUrl& url,
    const QUrl& embedder,
    scoped_ptr<oxide::PermissionRequest> request)
    : q_ptr(nullptr),
      request_(request.Pass()),
      is_cancelled_(false),
      url_(url),
      embedder_(embedder) {
}

OxideQPermissionRequestPrivate::~OxideQPermissionRequestPrivate() {}

OxideQSimplePermissionRequestPrivate::OxideQSimplePermissionRequestPrivate(
    const QUrl& url,
    const QUrl& embedder,
    scoped_ptr<oxide::SimplePermissionRequest> request)
    : OxideQPermissionRequestPrivate(
        url,
        embedder,
        request.Pass()),
      did_respond_(false) {}

bool OxideQSimplePermissionRequestPrivate::canRespond() const {
  if (did_respond_) {
    qWarning() << "Can only respond once to a permission request";
    return false;
  }

  if (is_cancelled_) {
    qWarning() << "Can't respond to a cancelled permission request";
    return false;
  }

  return true;
}

oxide::SimplePermissionRequest*
OxideQSimplePermissionRequestPrivate::request() const {
  return static_cast<oxide::SimplePermissionRequest *>(request_.get());
}

OxideQSimplePermissionRequestPrivate::~OxideQSimplePermissionRequestPrivate() {}

// static
OxideQSimplePermissionRequest* OxideQSimplePermissionRequestPrivate::Create(
    const QUrl& url,
    const QUrl& embedder,
    scoped_ptr<oxide::SimplePermissionRequest> request) {
  DCHECK(request);

  return new OxideQSimplePermissionRequest(
      *new OxideQSimplePermissionRequestPrivate(
        url,
        embedder,
        request.Pass()));
}

OxideQGeolocationPermissionRequestPrivate::OxideQGeolocationPermissionRequestPrivate(
    const QUrl& url,
    const QUrl& embedder,
    scoped_ptr<oxide::SimplePermissionRequest> request)
    : OxideQSimplePermissionRequestPrivate(url, embedder, request.Pass()) {}

OxideQGeolocationPermissionRequestPrivate::~OxideQGeolocationPermissionRequestPrivate() {}

// static
OxideQGeolocationPermissionRequest*
OxideQGeolocationPermissionRequestPrivate::Create(
    const QUrl& url,
    const QUrl& embedder,
    scoped_ptr<oxide::SimplePermissionRequest> request) {
  DCHECK(request);

  return new OxideQGeolocationPermissionRequest(
      *new OxideQGeolocationPermissionRequestPrivate(
        url,
        embedder,
        request.Pass()));
}

OxideQPermissionRequest::OxideQPermissionRequest(
    OxideQPermissionRequestPrivate& dd)
    : d_ptr(&dd) {
  Q_D(OxideQPermissionRequest);

  d->q_ptr = this;
  d->request_->SetCancelCallback(
      base::Bind(&OxideQPermissionRequestPrivate::OnCancelled,
                 base::Unretained(d)));
}

OxideQPermissionRequest::~OxideQPermissionRequest() {}

QUrl OxideQPermissionRequest::url() const {
  Q_D(const OxideQPermissionRequest);

  return d->url_;
}

QUrl OxideQPermissionRequest::embedder() const {
  Q_D(const OxideQPermissionRequest);

  return d->embedder_;
}

bool OxideQPermissionRequest::isCancelled() const {
  Q_D(const OxideQPermissionRequest);

  return d->is_cancelled_;
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

  d->did_respond_ = true;
  d->request()->Allow();
}

void OxideQSimplePermissionRequest::deny() {
  Q_D(OxideQSimplePermissionRequest);

  if (!d->canRespond()) {
    return;
  }

  d->did_respond_ = true;
  d->request()->Deny();
}

OxideQGeolocationPermissionRequest::OxideQGeolocationPermissionRequest(
    OxideQGeolocationPermissionRequestPrivate& dd)
    : OxideQSimplePermissionRequest(dd) {}

OxideQGeolocationPermissionRequest::~OxideQGeolocationPermissionRequest() {}

QUrl OxideQGeolocationPermissionRequest::origin() const {
  return url();
}

void OxideQGeolocationPermissionRequest::accept() {
  allow();
}
