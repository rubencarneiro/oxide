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

#include "oxideqquicktouchselectioncontroller_p.h"

#include "oxideqquickwebview_p_p.h"

class OxideQQuickTouchSelectionControllerPrivate {
 public:
  OxideQQuickTouchSelectionControllerPrivate()
      : view(nullptr)
      , active(false) {}

  OxideQQuickWebView* view;
  bool active;
  QRectF bounds;
};

OxideQQuickTouchSelectionController::OxideQQuickTouchSelectionController(
    OxideQQuickWebView* view)
    : d_ptr(new OxideQQuickTouchSelectionControllerPrivate()) {
  Q_D(OxideQQuickTouchSelectionController);

  d->view = view;
}

OxideQQuickTouchSelectionController::~OxideQQuickTouchSelectionController() {}

bool OxideQQuickTouchSelectionController::active() const {
  Q_D(const OxideQQuickTouchSelectionController);

  return d->active;
}

void OxideQQuickTouchSelectionController::setActive(bool active) {
  Q_D(OxideQQuickTouchSelectionController);

  if (active != d->active) {
    d->active = active;
    Q_EMIT activeChanged();
  }
}

QQmlComponent* OxideQQuickTouchSelectionController::handle() const {
  Q_D(const OxideQQuickTouchSelectionController);

  return OxideQQuickWebViewPrivate::get(d->view)->touchSelectionControllerHandle();
}

void OxideQQuickTouchSelectionController::setHandle(QQmlComponent* handle) {
  Q_D(OxideQQuickTouchSelectionController);

  if (handle == this->handle()) {
    return;
  }

  OxideQQuickWebViewPrivate::get(d->view)->setTouchSelectionControllerHandle(handle);
  Q_EMIT handleChanged();
}

const QRectF& OxideQQuickTouchSelectionController::bounds() const {
  Q_D(const OxideQQuickTouchSelectionController);

  return d->bounds;
}

void OxideQQuickTouchSelectionController::setBounds(const QRectF& bounds) {
  Q_D(OxideQQuickTouchSelectionController);

  if (bounds != d->bounds) {
    d->bounds = bounds;
    Q_EMIT boundsChanged();
  }
}
