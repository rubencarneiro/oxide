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

#include "oxideqquicklocationbarcontroller.h"
#include "oxideqquicklocationbarcontroller_p.h"

#include <QtDebug>

#include "qt/core/glue/chrome_controller.h"
#include "qt/core/glue/macros.h"

OxideQQuickLocationBarControllerPrivate
    ::OxideQQuickLocationBarControllerPrivate(
        OxideQQuickLocationBarController* q)
    : q_ptr(q),
      chrome_controller_(oxide::qt::ChromeController::create(this, q)) {}

void OxideQQuickLocationBarControllerPrivate::ChromePositionUpdated() {
  Q_Q(OxideQQuickLocationBarController);

  Q_EMIT q->offsetChanged();
  Q_EMIT q->contentOffsetChanged();
}

OxideQQuickLocationBarControllerPrivate
    ::~OxideQQuickLocationBarControllerPrivate() = default;

// static
std::unique_ptr<OxideQQuickLocationBarController>
OxideQQuickLocationBarControllerPrivate::create() {
  return std::unique_ptr<OxideQQuickLocationBarController>(
      new OxideQQuickLocationBarController());
}

// static
OxideQQuickLocationBarControllerPrivate*
OxideQQuickLocationBarControllerPrivate::get(
    OxideQQuickLocationBarController* q) {
  return q->d_func();
}

void OxideQQuickLocationBarControllerPrivate::init(
    oxide::qt::WebContentsID web_contents_id) {
  chrome_controller_->init(web_contents_id);
}

/*!
\class OxideQQuickLocationBarController
\inmodule OxideQtQuick
\inheaderfile oxideqquicklocationbarcontroller.h
\since OxideQt 1.5

\brief Bridge for location bar autohide functionality
*/

/*!
\qmltype LocationBarController
\inqmlmodule com.canonical.Oxide 1.5
\instantiates OxideQQuickLocationBarController
\since OxideQt 1.5

\brief Bridge for location bar autohide functionality

LocationBarContoller provides a bridge to allow applications to provide location
bar autohide functionality. It assumes that the location bar is at the top of
the view, and overlays the view when fully revealed.

LocationBarController works by calculating a position that the application
should use to position its location bar (via the \l{offset} property), and by
applying an appropriate offset to the web content and input events.

When in auto mode, this API allows applications to gradually reveal or hide
their location bar as content is scrolled with touch gestures. It does this by
consuming scroll gestures and calculating new location bar and content positions
(actually, this calculation is done by the renderer compositor).

The application must specify the fully revealed height of its location bar by
setting the \l{height} property.
*/

OxideQQuickLocationBarController::OxideQQuickLocationBarController()
    : d_ptr(new OxideQQuickLocationBarControllerPrivate(this)) {}

/*!
\internal
*/

OxideQQuickLocationBarController::~OxideQQuickLocationBarController() = default;

/*!
\qmlproperty real LocationBarController::height

The maximum height of the application's location bar. The default is 0.

Setting this to 0 effectively disables this API.
*/

qreal OxideQQuickLocationBarController::height() const {
  Q_D(const OxideQQuickLocationBarController);

  return d->chrome_controller_->topControlsHeight();
}

void OxideQQuickLocationBarController::setHeight(qreal height) {
  Q_D(OxideQQuickLocationBarController);

  if (height < 0.0f) {
    qWarning() <<
        "OxideQQuickLocationBarController: height cannot be negative";
    return;
  }

  if (height == this->height()) {
    return;
  }

  d->chrome_controller_->setTopControlsHeight(height);
  Q_EMIT heightChanged();
}

/*!
\qmlproperty enumeration LocationBarController::mode

The operational mode of the LocationBarController. Possible values are:

\value LocationBarController.ModeAuto
Auto hide and auto reveal of the location bar will be enabled.

\value LocationBarController.ModeShown
The location bar should be fully revealed.

\value LocationBarController.ModeHidden
The location bar should be fully hidden.

The default is \e{ModeAuto}.
*/

OxideQQuickLocationBarController::Mode
OxideQQuickLocationBarController::mode() const {
  Q_D(const OxideQQuickLocationBarController);

  STATIC_ASSERT_MATCHING_ENUM(ModeAuto,
                              oxide::qt::ChromeController::Mode::Auto);
  STATIC_ASSERT_MATCHING_ENUM(ModeShown,
                              oxide::qt::ChromeController::Mode::Shown);
  STATIC_ASSERT_MATCHING_ENUM(ModeHidden,
                              oxide::qt::ChromeController::Mode::Hidden);

  return static_cast<Mode>(d->chrome_controller_->mode());
}

void OxideQQuickLocationBarController::setMode(Mode mode) {
  Q_D(OxideQQuickLocationBarController);

  if (mode == this->mode()) {
    return;
  }

  d->chrome_controller_->setMode(
      static_cast<oxide::qt::ChromeController::Mode>(mode));
  Q_EMIT modeChanged();
}

/*!
\qmlproperty bool LocationBarController::animated
\since OxideQt 1.7

Whether transitions between different modes should be animated. The default
value is true.
*/

bool OxideQQuickLocationBarController::animated() const {
  Q_D(const OxideQQuickLocationBarController);

  return d->chrome_controller_->animationEnabled();
}

void OxideQQuickLocationBarController::setAnimated(bool animated) {
  Q_D(OxideQQuickLocationBarController);

  if (animated == this->animated()) {
    return;
  }

  d->chrome_controller_->setAnimationEnabled(animated);
  Q_EMIT animatedChanged();
}

/*!
\qmlproperty real LocationBarController::offset

This is the offset for the location bar, calculated by Oxide. The application
should use the value of this to position its location bar accordingly.

Possible values for this will range from 0 (fully revealed) to -\l{height}
(fully hidden).

The application should update the position of its location bar synchronously to
avoid tearing.
*/

qreal OxideQQuickLocationBarController::offset() const {
  Q_D(const OxideQQuickLocationBarController);

  return d->chrome_controller_->topControlsOffset();
}

/*!
\qmlproperty real LocationBarController::contentOffset

This is the offset of the web content and input events, calculated by Oxide.

Possible values for this will range from 0 (fully hidden) to \l{height} (fully
revealed).
*/

qreal OxideQQuickLocationBarController::contentOffset() const {
  Q_D(const OxideQQuickLocationBarController);

  return d->chrome_controller_->topContentOffset();
}

/*!
\qmlmethod void LocationBarController::show(bool animate)
\since OxideQt 1.7

In auto mode, calling this will request that the location bar is revealed. Once
it is revealed, auto hide will continue functioning as normal.

The application can control whether the transition is animated by setting the
\a{animate} argument appropriately.

Calling this in a mode other than auto will have no effect.
*/

void OxideQQuickLocationBarController::show(bool animate) {
  Q_D(OxideQQuickLocationBarController);

  if (mode() != ModeAuto) {
    qWarning() <<
        "OxideQQuickLocationBarController::show: mode is not ModeAuto";
    return;
  }

  if (height() <= 0.f) {
    qWarning() <<
        "OxideQQuickLocationBarController::show: height is not greater than "
        "zero";
    return;
  }

  d->chrome_controller_->show(animate);
}

/*!
\qmlmethod void LocationBarController::hide(bool animate)
\since OxideQt 1.7

In auto mode, calling this will request that the location bar is hidden. Once
it is hidden, auto reveal will continue functioning as normal.

The application can control whether the transition is animated by setting the
\a{animate} argument appropriately.

Calling this in a mode other than auto will have no effect.
*/

void OxideQQuickLocationBarController::hide(bool animate) {
  Q_D(OxideQQuickLocationBarController);

  if (mode() != ModeAuto) {
    qWarning() <<
        "OxideQQuickLocationBarController::hide: mode is not ModeAuto";
    return;
  }

  if (height() <= 0.f) {
    qWarning() <<
        "OxideQQuickLocationBarController::hide: height is not greater than "
        "zero";
    return;
  }

  d->chrome_controller_->hide(animate);
}
