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

#include "oxideqnewviewrequest.h"
#include "oxideqnewviewrequest_p.h"

#include "content/public/browser/web_contents.h"

OxideQNewViewRequestPrivate::OxideQNewViewRequestPrivate(
    const QRect& position,
    OxideQNewViewRequest::Disposition disposition) :
    position_(position),
    disposition_(disposition) {}

OxideQNewViewRequestPrivate::~OxideQNewViewRequestPrivate() {}

// static
OxideQNewViewRequestPrivate* OxideQNewViewRequestPrivate::get(
    OxideQNewViewRequest* q) {
  return q->d_func();
}

OxideQNewViewRequest::OxideQNewViewRequest(const QRect& position,
                                           Disposition disposition) :
    d_ptr(new OxideQNewViewRequestPrivate(position, disposition)) {}

OxideQNewViewRequest::~OxideQNewViewRequest() {}

QRect OxideQNewViewRequest::position() const {
  Q_D(const OxideQNewViewRequest);

  return d->position_;
}

QRectF OxideQNewViewRequest::positionF() const {
  Q_D(const OxideQNewViewRequest);

  return d->position_;
}

OxideQNewViewRequest::Disposition OxideQNewViewRequest::disposition() const {
  Q_D(const OxideQNewViewRequest);

  return d->disposition_;
}
