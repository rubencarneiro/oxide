// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015-2016 Canonical Ltd.

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

#include "oxideqquicktouchselectioncontroller.h"
#include "oxideqquicktouchselectioncontroller_p.h"

#include <QQmlComponent>

#include "qt/core/api/oxideqglobal_p.h"
#include "qt/core/glue/legacy_touch_editing_controller.h"

QT_BEGIN_NAMESPACE
class QQmlComponent;
QT_END_NAMESPACE

OxideQQuickTouchSelectionControllerPrivate
    ::OxideQQuickTouchSelectionControllerPrivate(
        OxideQQuickTouchSelectionController* q)
        : q_ptr(q),
          handle_drag_in_progress_(false),
          status_(OxideQQuickTouchSelectionController::StatusInactive) {}

void OxideQQuickTouchSelectionControllerPrivate::StatusChanged(
    ActiveStatus status,
    const QRectF& bounds,
    bool handle_drag_in_progress) {
  Q_Q(OxideQQuickTouchSelectionController);

  OxideQQuickTouchSelectionController::Status s =
      static_cast<OxideQQuickTouchSelectionController::Status>(status);
  if (s != status_) {
    bool was_active =
        (status_ != OxideQQuickTouchSelectionController::StatusInactive);
    status_ = s;
    Q_EMIT q->statusChanged();
    bool active =
        (s != OxideQQuickTouchSelectionController::StatusInactive);
    if (active != was_active) {
      Q_EMIT q->activeChanged();
    }
  }

  if (bounds != bounds_) {
    bounds_ = bounds;
    Q_EMIT q->boundsChanged();
  }

  if (handle_drag_in_progress != handle_drag_in_progress_) {
    handle_drag_in_progress_ = handle_drag_in_progress;
    Q_EMIT q->handleDragInProgressChanged();
  }
}

void OxideQQuickTouchSelectionControllerPrivate::InsertionHandleTapped() {
  Q_Q(OxideQQuickTouchSelectionController);

  Q_EMIT q->insertionHandleTapped();
}

void OxideQQuickTouchSelectionControllerPrivate::ContextMenuIntercepted() {
  Q_Q(OxideQQuickTouchSelectionController);

  Q_EMIT q->contextMenuIntercepted();
}

// static
OxideQQuickTouchSelectionControllerPrivate*
OxideQQuickTouchSelectionControllerPrivate::get(
    OxideQQuickTouchSelectionController* q) {
  return q->d_func();
}

/*!
\class OxideQQuickTouchSelectionController
\inmodule OxideQtQuick
\inheaderfile oxideqquicktouchselectioncontroller.h

\brief Controller for coordinating a touch selection UI
*/

/*!
\qmltype TouchSelectionController
\inqmlmodule com.canonical.Oxide 1.12
\instantiates OxideQQuickTouchSelectionController
\since OxideQt 1.12

\brief Controller for coordinating a touch selection UI
*/

/*!
\qmlsignal void TouchSelectionController::insertionHandleTapped()
\since OxideQt 1.15
*/

/*!
\qmlsignal void TouchSelectionController::contextMenuIntercepted()
\since OxideQt 1.15
*/

/*!
\internal
*/

OxideQQuickTouchSelectionController::OxideQQuickTouchSelectionController()
    : d_ptr(new OxideQQuickTouchSelectionControllerPrivate(this)) {}

/*!
\internal
*/

OxideQQuickTouchSelectionController::~OxideQQuickTouchSelectionController() {}

/*!
\qmlmethod void TouchSelectionController::hide()
\since OxideQt 1.15
*/

void OxideQQuickTouchSelectionController::hide() const {
  Q_D(const OxideQQuickTouchSelectionController);

  d->controller()->HideAndDisallowShowingAutomatically();
}

/*!
\qmlproperty bool TouchSelectionController::active
\deprecated
*/

bool OxideQQuickTouchSelectionController::active() const {
  Q_D(const OxideQQuickTouchSelectionController);

  WARN_DEPRECATED_API_USAGE() <<
      "TouchSelectionController::active is deprecated, use "
      "TouchSelectionController::status instead";

  return (d->status_ != StatusInactive);
}

/*!
\qmlproperty Component TouchSelectionController::handle
*/

QQmlComponent* OxideQQuickTouchSelectionController::handle() const {
  Q_D(const OxideQQuickTouchSelectionController);

  return d->handle_;
}

void OxideQQuickTouchSelectionController::setHandle(QQmlComponent* handle) {
  Q_D(OxideQQuickTouchSelectionController);

  if (handle == d->handle_) {
    return;
  }

  d->handle_ = handle;
  Q_EMIT handleChanged();
}

/*!
\qmlproperty rect TouchSelectionController::bounds
*/

const QRectF& OxideQQuickTouchSelectionController::bounds() const {
  Q_D(const OxideQQuickTouchSelectionController);

  return d->bounds_;
}

/*!
\qmlproperty bool TouchSelectionController::handleDragInProgress
\since OxideQt 1.15
*/

bool OxideQQuickTouchSelectionController::handleDragInProgress() const {
  Q_D(const OxideQQuickTouchSelectionController);

  return d->handle_drag_in_progress_;
}

/*!
\qmlproperty enumeration TouchSelectionController::status
\since OxideQt 1.15
*/

OxideQQuickTouchSelectionController::Status
OxideQQuickTouchSelectionController::status() const {
  Q_D(const OxideQQuickTouchSelectionController);

  return d->status_;
}
