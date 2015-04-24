// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#include "oxidequseragentoverriderequest_p.h"
#include "oxidequseragentoverriderequest_p_p.h"

OxideQUserAgentOverrideRequestPrivate::OxideQUserAgentOverrideRequestPrivate(
    const QUrl& url) :
    url_(url) {}

OxideQUserAgentOverrideRequestPrivate::~OxideQUserAgentOverrideRequestPrivate() {}

// static
OxideQUserAgentOverrideRequestPrivate* OxideQUserAgentOverrideRequestPrivate::get(
    OxideQUserAgentOverrideRequest* q) {
  return q->d_func();
}

OxideQUserAgentOverrideRequest::~OxideQUserAgentOverrideRequest() {}

OxideQUserAgentOverrideRequest::OxideQUserAgentOverrideRequest(
    const QUrl& url) :
    d_ptr(new OxideQUserAgentOverrideRequestPrivate(url)) {}

QUrl OxideQUserAgentOverrideRequest::url() const {
  Q_D(const OxideQUserAgentOverrideRequest);

  return d->url_;
}

QString OxideQUserAgentOverrideRequest::userAgentOverride() const {
  Q_D(const OxideQUserAgentOverrideRequest);

  return d->user_agent;
}

void OxideQUserAgentOverrideRequest::setUserAgentOverride(
    const QString& user_agent) {
  Q_D(OxideQUserAgentOverrideRequest);

  d->user_agent = user_agent;
}
