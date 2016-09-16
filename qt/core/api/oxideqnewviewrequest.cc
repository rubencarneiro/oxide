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

/*!
\class OxideQNewViewRequest
\inmodule OxideQtCore
\inheaderfile oxideqnewviewrequest.h

\brief Request to open a new view

OxideQNewViewRequest represents a request to open a new view. \l{disposition}
provides a hint for the type of view that the application should create, and
\l{position} provides a hint for the position of the new view.

Applications should respond to this by creating a new view with this request as
a construct parameter.
*/

/*!
\enum OxideQNewViewRequest::Disposition

\omitvalue DispositionCurrentTab

\value DispositionNewForegroundTab
The new view should be presented as a new foreground tab

\value DispositionNewBackgroundTab
The new view should be presented as a new background tab

\value DispositionNewPopup
The new view should be presented as a popup. This generally means a new window
with reduced UI

\value DispositionNewWindow
The new view should be presented in a new window.
*/

OxideQNewViewRequest::OxideQNewViewRequest(const QRect& position,
                                           Disposition disposition) :
    d_ptr(new OxideQNewViewRequestPrivate(position, disposition)) {}

/*!
\internal
*/

OxideQNewViewRequest::~OxideQNewViewRequest() {}

/*!
Returns a position hint for the new view, in screen coordinates.
*/

QRect OxideQNewViewRequest::position() const {
  Q_D(const OxideQNewViewRequest);

  return d->position_;
}

/*!
\property OxideQNewViewRequest::position

Position hint for the new view, in screen coordinates.
*/

QRectF OxideQNewViewRequest::positionF() const {
  Q_D(const OxideQNewViewRequest);

  return d->position_;
}

/*!
\property OxideQNewViewRequest::disposition

Disposition hint for the new view. This is a hint to the application for how the
new view should be presented.
*/

OxideQNewViewRequest::Disposition OxideQNewViewRequest::disposition() const {
  Q_D(const OxideQNewViewRequest);

  return d->disposition_;
}
