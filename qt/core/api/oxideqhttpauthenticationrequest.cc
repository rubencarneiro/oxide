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

#include "shared/browser/oxide_resource_dispatcher_host_login_delegate.h"
#include "qt/core/browser/oxide_qt_web_view.h"

#include "oxideqhttpauthenticationrequest.h"
#include "oxideqhttpauthenticationrequest_p.h"

OxideQHttpAuthenticationRequestPrivate::OxideQHttpAuthenticationRequestPrivate(
    OxideQHttpAuthenticationRequest* q,
    oxide::ResourceDispatcherHostLoginDelegate* login_delegate)
    : q_ptr(q),
      login_delegate_(login_delegate),
      host_(QString::fromStdString(login_delegate->Host())),
      realm_(QString::fromStdString(login_delegate->Realm())) {
  // Use of base::Unretained is safe here because we clear the callback
  // in the destructor, so that it can not called back on a deleted object
  login_delegate->SetCancelledCallback(
        base::Bind(&OxideQHttpAuthenticationRequestPrivate::RequestCancelled,
                   base::Unretained(this)));
}

OxideQHttpAuthenticationRequestPrivate::~OxideQHttpAuthenticationRequestPrivate() {
  if (login_delegate_) {
    login_delegate_->SetCancelledCallback(base::Closure());

    // Always deny the request when we are being destroyed, as it should be the
    // default action in case the client ignores this request and it gets
    // garbage collected.
    // In case an action has already been taken, this is a essentially a no-op.
    login_delegate_->Deny();
  }
}

// static
OxideQHttpAuthenticationRequest* OxideQHttpAuthenticationRequestPrivate::Create(
    oxide::ResourceDispatcherHostLoginDelegate* login_delegate) {
  DCHECK(login_delegate);

  OxideQHttpAuthenticationRequest* req = new OxideQHttpAuthenticationRequest();
  req->d_ptr.reset(new OxideQHttpAuthenticationRequestPrivate(req,
                                                              login_delegate));
  return req;
}

void OxideQHttpAuthenticationRequestPrivate::RequestCancelled() {
  Q_Q(OxideQHttpAuthenticationRequest);

  login_delegate_ = nullptr;
  q->cancelled();
}

OxideQHttpAuthenticationRequest::OxideQHttpAuthenticationRequest() {}

OxideQHttpAuthenticationRequest::~OxideQHttpAuthenticationRequest() {}

QString OxideQHttpAuthenticationRequest::host() const {
  Q_D(const OxideQHttpAuthenticationRequest);

  return d->host_;
}

QString OxideQHttpAuthenticationRequest::realm() const {
  Q_D(const OxideQHttpAuthenticationRequest);

  return d->realm_;
}

void OxideQHttpAuthenticationRequest::deny() {
  Q_D(OxideQHttpAuthenticationRequest);

  if (d->login_delegate_) {
    d->login_delegate_->Deny();
    d->login_delegate_ = nullptr;
  }
}

void OxideQHttpAuthenticationRequest::allow(const QString &username,
                                             const QString &password) {
  Q_D(OxideQHttpAuthenticationRequest);

  if (d->login_delegate_) {
    d->login_delegate_->Allow(username.toStdString(), password.toStdString());
    d->login_delegate_ = nullptr;
  }
}
