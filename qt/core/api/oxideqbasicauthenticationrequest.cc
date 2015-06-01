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

#include "oxideqbasicauthenticationrequest.h"
#include "oxideqbasicauthenticationrequest_p.h"

#include "shared/browser/oxide_resource_dispatcher_host_delegate.h"
#include "qt/core/browser/oxide_qt_web_view.h"
#include "shared/browser/oxide_web_view.h"

OxideQBasicAuthenticationRequestPrivate::OxideQBasicAuthenticationRequestPrivate
    (oxide::ResourceDispatcherHostDelegate* resource_dispatcher_host_delegate) :
    resource_dispatcher_host_delegate_(resource_dispatcher_host_delegate) {}

OxideQBasicAuthenticationRequestPrivate::~OxideQBasicAuthenticationRequestPrivate
    () {}

OxideQBasicAuthenticationRequest::OxideQBasicAuthenticationRequest(
    oxide::ResourceDispatcherHostDelegate* resource_dispatcher_host_delegate) :
    QObject(nullptr),
    d_ptr(new OxideQBasicAuthenticationRequestPrivate(
              resource_dispatcher_host_delegate)) {
}

#include <QDebug>
OxideQBasicAuthenticationRequest::~OxideQBasicAuthenticationRequest() {
    qWarning() << "QRequest deleted ++++++++++++++++++++++++++";
}

QString OxideQBasicAuthenticationRequest::realm() const {
  Q_D(const OxideQBasicAuthenticationRequest);

  return d->realm_;
}

void OxideQBasicAuthenticationRequest::deny() {
  Q_D(OxideQBasicAuthenticationRequest);

  d->resource_dispatcher_host_delegate_->CancelAuthentication();
}

void OxideQBasicAuthenticationRequest::allow(const QString &username,
                                             const QString &password) {
  Q_D(OxideQBasicAuthenticationRequest);

  d->resource_dispatcher_host_delegate_->SendAuthenticationCredentials(
              username.toStdString(), password.toStdString());
}
