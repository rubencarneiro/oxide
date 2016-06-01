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

#include "qt/core/api/oxideqglobal_p.h"
#include "qt/quick/oxide_qquick_contents_view.h"

#include "oxideqquickwebview_p.h"

QT_BEGIN_NAMESPACE
class QQmlComponent;
QT_END_NAMESPACE

class OxideQQuickTouchSelectionControllerPrivate {
 public:
  OxideQQuickTouchSelectionControllerPrivate()
      : view(nullptr)
      , handle(nullptr)
      , handle_drag_in_progress(false)
      , status(OxideQQuickTouchSelectionController::StatusInactive) {}

  oxide::qquick::ContentsView* view;
  QQmlComponent* handle;
  QRectF bounds;
  bool handle_drag_in_progress;
  OxideQQuickTouchSelectionController::Status status;
};

OxideQQuickTouchSelectionController::OxideQQuickTouchSelectionController(
    oxide::qquick::ContentsView* view)
    : d_ptr(new OxideQQuickTouchSelectionControllerPrivate()) {
  Q_D(OxideQQuickTouchSelectionController);

  d->view = view;
}

OxideQQuickTouchSelectionController::~OxideQQuickTouchSelectionController() {}

void OxideQQuickTouchSelectionController::hide() const {
  Q_D(const OxideQQuickTouchSelectionController);

  d->view->hideTouchSelectionController();
}

bool OxideQQuickTouchSelectionController::active() const {
  Q_D(const OxideQQuickTouchSelectionController);

  WARN_DEPRECATED_API_USAGE() <<
      "TouchSelectionController::active is deprecated, use "
      "TouchSelectionController::status instead";

  return (d->status != StatusInactive);
}

QQmlComponent* OxideQQuickTouchSelectionController::handle() const {
  Q_D(const OxideQQuickTouchSelectionController);

  return d->handle;
}

void OxideQQuickTouchSelectionController::setHandle(QQmlComponent* handle) {
  Q_D(OxideQQuickTouchSelectionController);

  if (handle == d->handle) {
    return;
  }

  d->handle = handle;
  Q_EMIT handleChanged();
}

const QRectF& OxideQQuickTouchSelectionController::bounds() const {
  Q_D(const OxideQQuickTouchSelectionController);

  return d->bounds;
}

bool OxideQQuickTouchSelectionController::handleDragInProgress() const {
  Q_D(const OxideQQuickTouchSelectionController);

  return d->handle_drag_in_progress;
}

OxideQQuickTouchSelectionController::Status
OxideQQuickTouchSelectionController::status() const {
  Q_D(const OxideQQuickTouchSelectionController);

  return d->status;
}

void OxideQQuickTouchSelectionController::onTouchSelectionChanged(
    Status status,
    const QRectF& bounds,
    bool handle_drag_in_progress,
    bool insertion_handle_tapped) {
  Q_D(OxideQQuickTouchSelectionController);

  if (status != d->status) {
    bool was_active = (d->status != StatusInactive);
    d->status = status;
    Q_EMIT statusChanged();
    bool active = (status != StatusInactive);
    if (active != was_active) {
      Q_EMIT activeChanged();
    }
  }

  if (bounds != d->bounds) {
    d->bounds = bounds;
    Q_EMIT boundsChanged();
  }

  if (handle_drag_in_progress != d->handle_drag_in_progress) {
    d->handle_drag_in_progress = handle_drag_in_progress;
    Q_EMIT handleDragInProgressChanged();
  }

  if (insertion_handle_tapped) {
    Q_EMIT insertionHandleTapped();
  }
}
