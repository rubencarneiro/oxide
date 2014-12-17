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

#include "oxideqquicklocationbarcontroller_p.h"

#include "qt/core/glue/oxide_qt_web_view_adapter.h"

#include "oxideqquickwebview_p.h"
#include "oxideqquickwebview_p_p.h"

class OxideQQuickLocationBarControllerPrivate {
 public:
  OxideQQuickLocationBarControllerPrivate()
      : view(NULL),
        max_height(0.0f),
        height(0.0f) {}

  OxideQQuickWebView* view;
  qreal max_height;
  qreal height;
};

OxideQQuickLocationBarController::OxideQQuickLocationBarController(
    OxideQQuickWebView* view)
    : d_ptr(new OxideQQuickLocationBarControllerPrivate()) {
  Q_D(OxideQQuickLocationBarController);

  d->view = view;
}

OxideQQuickLocationBarController::~OxideQQuickLocationBarController() {}

qreal OxideQQuickLocationBarController::maxHeight() const {
  Q_D(const OxideQQuickLocationBarController);

  OxideQQuickWebViewPrivate* p = OxideQQuickWebViewPrivate::get(d->view);
  if (p->isInitialized()) {
    return p->locationBarMaxHeight();
  }

  return d->max_height;
}

void OxideQQuickLocationBarController::setMaxHeight(qreal height) {
  Q_D(OxideQQuickLocationBarController);

  if (OxideQQuickWebViewPrivate::get(d->view)->isInitialized()) {
    qWarning() << "LocationBarController.maxheight must be set during construction";
    return;
  }

  if (height < 0.0f) {
    qWarning() << "LocationBarController.maxHeight cannot be negative";
    return;
  }

  if (height == d->max_height) {
    return;
  }

  d->max_height = height;
  Q_EMIT maxHeightChanged();
}

OxideQQuickLocationBarController::Mode
OxideQQuickLocationBarController::mode() const {
  Q_D(const OxideQQuickLocationBarController);

  Q_STATIC_ASSERT(
      static_cast<int>(ModeAuto) ==
        static_cast<int>(oxide::qt::LOCATION_BAR_MODE_AUTO));
  Q_STATIC_ASSERT(
      static_cast<int>(ModeShown) ==
        static_cast<int>(oxide::qt::LOCATION_BAR_MODE_SHOWN));
  Q_STATIC_ASSERT(
      static_cast<int>(ModeHidden) ==
        static_cast<int>(oxide::qt::LOCATION_BAR_MODE_HIDDEN));

  return static_cast<Mode>(
      OxideQQuickWebViewPrivate::get(d->view)->locationBarMode());
}

void OxideQQuickLocationBarController::setMode(Mode mode) {
  Q_D(OxideQQuickLocationBarController);

  if (mode == this->mode()) {
    return;
  }

  if (maxHeight() == 0) {
    return;
  }

  OxideQQuickWebViewPrivate::get(d->view)->setLocationBarMode(
      static_cast<oxide::qt::LocationBarMode>(mode));
  Q_EMIT modeChanged();
}

qreal OxideQQuickLocationBarController::offset() const {
  Q_D(const OxideQQuickLocationBarController);

  return OxideQQuickWebViewPrivate::get(d->view)->locationBarOffsetPix();
}

qreal OxideQQuickLocationBarController::contentOffset() const {
  Q_D(const OxideQQuickLocationBarController);

  return OxideQQuickWebViewPrivate::get(
      d->view)->locationBarContentOffsetPix();
}

qreal OxideQQuickLocationBarController::height() const {
  Q_D(const OxideQQuickLocationBarController);

  return d->height;
}

void OxideQQuickLocationBarController::setHeight(qreal height) {
  Q_D(OxideQQuickLocationBarController);

  if (height == d->height) {
    return;
  }

  d->height = height;

  OxideQQuickWebViewPrivate* p = OxideQQuickWebViewPrivate::get(d->view);
  if (p->isInitialized() && d->view->window()) {
    p->wasResized();
  }

  Q_EMIT heightChanged();
}
