// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#include "base/bind.h"
#include "base/callback.h"

#include "shared/browser/oxide_resource_dispatcher_host_delegate.h"
#include "qt/core/browser/oxide_qt_web_view.h"

#include "oxideqhttpauthenticationrequest.h"
#include "oxideqhttpauthenticationrequest_p.h"

OxideQHttpAuthenticationRequestPrivate::OxideQHttpAuthenticationRequestPrivate(
    oxide::ResourceDispatcherHostLoginDelegate* login_delegate)
    : q_ptr(nullptr),
      login_delegate_(login_delegate) {
  // Use of base::Unretained is safe here because we clear the callback in the
  // destructor, so that the login delegate can not call it anymore
  login_delegate->SetCancelledCallback(
        base::Bind(&OxideQHttpAuthenticationRequestPrivate::RequestCancelled,
                   base::Unretained(this)));
}

OxideQHttpAuthenticationRequestPrivate::~OxideQHttpAuthenticationRequestPrivate() {
}

void OxideQHttpAuthenticationRequestPrivate::RequestCancelled() {
  Q_Q(OxideQHttpAuthenticationRequest);

  login_delegate_ = nullptr;
  q->cancelled();
}

OxideQHttpAuthenticationRequest::OxideQHttpAuthenticationRequest(
    oxide::ResourceDispatcherHostLoginDelegate* login_delegate)
    : d_ptr(new OxideQHttpAuthenticationRequestPrivate(login_delegate)) {
  Q_D(OxideQHttpAuthenticationRequest);
  d->q_ptr = this;
}

OxideQHttpAuthenticationRequest::~OxideQHttpAuthenticationRequest() {
  Q_D(OxideQHttpAuthenticationRequest);

  d->login_delegate_->SetCancelledCallback(base::Closure());
}

QString OxideQHttpAuthenticationRequest::host() const {
  Q_D(const OxideQHttpAuthenticationRequest);

  if (d->login_delegate_) {
    return QString::fromStdString(d->login_delegate_->Host());
  } else {
    return QString();
  }
}

QString OxideQHttpAuthenticationRequest::realm() const {
  Q_D(const OxideQHttpAuthenticationRequest);

  if (d->login_delegate_) {
    return QString::fromStdString(d->login_delegate_->Realm());
  } else {
    return QString();
  }
}

void OxideQHttpAuthenticationRequest::deny() {
  Q_D(OxideQHttpAuthenticationRequest);

  if (d->login_delegate_) {
    d->login_delegate_->Deny();
  }
}

void OxideQHttpAuthenticationRequest::allow(const QString &username,
                                             const QString &password) {
  Q_D(OxideQHttpAuthenticationRequest);

  if (d->login_delegate_) {
    d->login_delegate_->Allow(username.toStdString(), password.toStdString());
  }
}
