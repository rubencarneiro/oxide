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

#include "oxideqnavigationrequest.h"

class OxideQNavigationRequestPrivate {
 public:
  ~OxideQNavigationRequestPrivate() {}

 private:
  friend class OxideQNavigationRequest;

  OxideQNavigationRequestPrivate(
      const QUrl& url,
      OxideQNavigationRequest::Disposition disposition,
      bool user_gesture) :
      url_(url), disposition_(disposition), user_gesture_(user_gesture),
      action_(OxideQNavigationRequest::ActionAccept) {}

  QUrl url_;
  OxideQNavigationRequest::Disposition disposition_;
  bool user_gesture_;
  OxideQNavigationRequest::Action action_;
};

/*!
\class OxideQNavigationRequest
\inmodule OxideQtCore
\inheaderfile oxideqnavigationrequest.h

\brief Request to navigate to a new page

OxideQNavigationRequest represents a request to navigate to a new page. It is
only used for content-initiated navigations.

Due to a mis-design of this API, it is currently used in more than one context.
If \l{disposition} is DispositionCurrentTab, then this is a request to
navigate the current webview. In this case \l{url} will indicate the actual URL
that will be committed (redirects have already occurred at this point).

If \l{disposition} is not DispositionCurrentTab, then this is actually part of
a request to open a new webview, whether this is due to a call to
\e{window.open()} or the result of a link click with modifier keys pressed. In
this case, \l{url} will indicate the initial URL that will be loaded in the new
view (before any redirects occur).

The application gives its response by setting \l{action} appropriately.
*/

/*!
\enum OxideQNavigationRequest::Disposition

\value DispositionCurrentTab
A request to navigate in the current view

\value DispositionNewForegroundTab
A request to begin a navigation in a new foreground tab

\value DispositionNewBackgroundTab
A request to begin a navigation in a new background tab

\value DispositionNewPopup
A request to begin a navigation in a popup

\value DispositionNewWindow
A request to begin a navigation in a new window
*/

/*!
\enum OxideQNavigationRequest::Action

\value ActionAccept
Allow the navigation to proceed

\value ActionReject
Block the navigation
*/

OxideQNavigationRequest::OxideQNavigationRequest(const QUrl& url,
                                                 Disposition disposition,
                                                 bool user_gesture) :
    d_ptr(new OxideQNavigationRequestPrivate(url, disposition, user_gesture)) {}

/*!
\internal
*/

OxideQNavigationRequest::~OxideQNavigationRequest() {}

/*!
\property OxideQNavigationRequest::url

The URL of the navigation request. If \l{disposition} is DispositionCurrentTab,
this is the URL that will be committed (redirects have already occurred), else
this will be the initial URL (before any redirects occur).
*/

QUrl OxideQNavigationRequest::url() const {
  Q_D(const OxideQNavigationRequest);

  return d->url_;
}

/*!
\property OxideQNavigationRequest::disposition

Indicates the type of request. If this is DispositionCurrentTab, then it is a
request to perform a navigation in the current view. Otherwise it is part of a
request to open a new view, with the disposition acting as a hint to the type of
view that the application will be asked to present.
*/

OxideQNavigationRequest::Disposition
OxideQNavigationRequest::disposition() const {
  Q_D(const OxideQNavigationRequest);

  return d->disposition_;
}

/*!
\property OxideQNavigationRequest::userGesture
\deprecated

Whether the navigation request was a result of a user gesture (eg, a tap or
mouse click).

\note This property doesn't work correctly in all circumstances.
*/

bool OxideQNavigationRequest::userGesture() const {
  Q_D(const OxideQNavigationRequest);

  return d->user_gesture_;
}

/*!
\property OxideQNavigationRequest::action

This property stores the application's response. The default is ActionAccept.
*/

OxideQNavigationRequest::Action OxideQNavigationRequest::action() const {
  Q_D(const OxideQNavigationRequest);

  return d->action_;
}

void OxideQNavigationRequest::setAction(Action action) {
  Q_D(OxideQNavigationRequest);

  if (action == d->action_) {
    return;
  }

  d->action_ = action;
  Q_EMIT actionChanged();
}
