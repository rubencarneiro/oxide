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

class Q_DECL_FINAL OxideQNavigationRequestPrivate {
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

OxideQNavigationRequest::~OxideQNavigationRequest() {}

OxideQNavigationRequest::OxideQNavigationRequest(const QUrl& url,
                                                 Disposition disposition,
                                                 bool user_gesture) :
    d_ptr(new OxideQNavigationRequestPrivate(url, disposition, user_gesture)) {}

QUrl OxideQNavigationRequest::url() const {
  Q_D(const OxideQNavigationRequest);

  return d->url_;
}

OxideQNavigationRequest::Disposition
OxideQNavigationRequest::disposition() const {
  Q_D(const OxideQNavigationRequest);

  return d->disposition_;
}

bool OxideQNavigationRequest::userGesture() const {
  Q_D(const OxideQNavigationRequest);

  return d->user_gesture_;
}

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
