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

#include "oxideqbasicauthenticationrequest.h"
#include "oxideqbasicauthenticationrequest_p.h"

OxideQBasicAuthenticationRequestPrivate::OxideQBasicAuthenticationRequestPrivate
    (oxide::ResourceDispatcherHostLoginDelegate* login_delegate) :
    q_ptr(nullptr),
    login_delegate_(login_delegate) {
    if (login_delegate) {
        login_delegate->SetCancelledCallback(
            base::Bind(&OxideQBasicAuthenticationRequestPrivate::RequestCancelled,
                       base::Unretained(this)));
    }
}

OxideQBasicAuthenticationRequestPrivate::~OxideQBasicAuthenticationRequestPrivate
    () {}

void OxideQBasicAuthenticationRequestPrivate::RequestCancelled() {
  Q_Q(OxideQBasicAuthenticationRequest);

  q->cancelled();
}

OxideQBasicAuthenticationRequest::OxideQBasicAuthenticationRequest(
    oxide::ResourceDispatcherHostLoginDelegate* login_delegate) :
    QObject(nullptr),
    d_ptr(new OxideQBasicAuthenticationRequestPrivate(
              login_delegate)) {
    Q_D(OxideQBasicAuthenticationRequest);
    d->q_ptr = this;
}

OxideQBasicAuthenticationRequest::~OxideQBasicAuthenticationRequest() {}

QString OxideQBasicAuthenticationRequest::host() const {
  Q_D(const OxideQBasicAuthenticationRequest);

  return QString::fromStdString(d->login_delegate_->Host());
}

QString OxideQBasicAuthenticationRequest::realm() const {
  Q_D(const OxideQBasicAuthenticationRequest);

  return QString::fromStdString(d->login_delegate_->Realm());
}

void OxideQBasicAuthenticationRequest::deny() {
  Q_D(OxideQBasicAuthenticationRequest);

  d->login_delegate_->Deny();
}

void OxideQBasicAuthenticationRequest::allow(const QString &username,
                                             const QString &password) {
  Q_D(OxideQBasicAuthenticationRequest);

  d->login_delegate_->Allow(username.toStdString(), password.toStdString());
}
