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
#include "url/gurl.h"

#include "shared/browser/oxide_permission_request.h"

void OxideQPermissionRequestPrivate::OnCancelled() {
  Q_Q(OxideQPermissionRequest);

  Q_EMIT q->cancelled();
}

OxideQPermissionRequestPrivate::OxideQPermissionRequestPrivate(
    OxideQPermissionRequest* q)
    : q_ptr(q) {}

OxideQPermissionRequestPrivate::~OxideQPermissionRequestPrivate() {}

// static
OxideQPermissionRequestPrivate* OxideQPermissionRequestPrivate::get(
    OxideQPermissionRequest* q) {
  return q->d_func();
}

void OxideQPermissionRequestPrivate::Init(
    scoped_ptr<oxide::PermissionRequest> request) {
  request_ = request.Pass();
  request_->SetCancelCallback(
      base::Bind(&OxideQPermissionRequestPrivate::OnCancelled,
                 base::Unretained(this)));
}

class OxideQGeolocationPermissionRequestPrivate :
    public OxideQPermissionRequestPrivate {
 public:
  virtual ~OxideQGeolocationPermissionRequestPrivate() {}

 private:
  friend class OxideQGeolocationPermissionRequest;

  oxide::GeolocationPermissionRequest* request() {
    return static_cast<oxide::GeolocationPermissionRequest *>(request_.get());
  }

  OxideQGeolocationPermissionRequestPrivate(
      OxideQGeolocationPermissionRequest* q)
      : OxideQPermissionRequestPrivate(q) {}
};

OxideQPermissionRequest::OxideQPermissionRequest(
    OxideQPermissionRequestPrivate& dd)
    : d_ptr(&dd) {}

OxideQPermissionRequest::~OxideQPermissionRequest() {}

QUrl OxideQPermissionRequest::origin() const {
  Q_D(const OxideQPermissionRequest);

  return QUrl(QString::fromStdString(d->request_->origin().spec()));
}

QUrl OxideQPermissionRequest::embedder() const {
  Q_D(const OxideQPermissionRequest);

  return QUrl(QString::fromStdString(d->request_->embedder().spec()));
}

bool OxideQPermissionRequest::isCancelled() const {
  Q_D(const OxideQPermissionRequest);

  return d->request_->is_cancelled();
}

OxideQGeolocationPermissionRequest::OxideQGeolocationPermissionRequest()
    : OxideQPermissionRequest(*new OxideQGeolocationPermissionRequestPrivate(this)) {}

OxideQGeolocationPermissionRequest::~OxideQGeolocationPermissionRequest() {}

void OxideQGeolocationPermissionRequest::accept() {
  Q_D(OxideQGeolocationPermissionRequest);

  if (d->request_->did_respond()) {
    qWarning() << "Can only respond once to a geolocation permission request";
    return;
  }

  if (isCancelled()) {
    qWarning() << "Can't respond to a cancelled geolocation permission request";
    return;
  }

  d->request()->Accept();
}

void OxideQGeolocationPermissionRequest::deny() {
  Q_D(OxideQGeolocationPermissionRequest);

  if (d->request_->did_respond()) {
    qWarning() << "Can only respond once to a geolocation permission request";
    return;
  }

  if (isCancelled()) {
    qWarning() << "Can't respond to a cancelled geolocation permission request";
    return;
  }

  d->request()->Deny();
}

