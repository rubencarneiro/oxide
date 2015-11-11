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
      , active(false)
      , edit_flags(OxideQQuickWebView::NoCapability) {}

  OxideQQuickWebView* view;
  bool active;
  OxideQQuickWebView::EditCapabilities edit_flags;
  QString selected_text;
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

OxideQQuickWebView::EditCapabilities
OxideQQuickTouchSelectionController::editFlags() const {
  Q_D(const OxideQQuickTouchSelectionController);

  return d->edit_flags;
}

void OxideQQuickTouchSelectionController::setEditFlags(
    OxideQQuickWebView::EditCapabilities flags) {
  Q_D(OxideQQuickTouchSelectionController);

  if (flags != d->edit_flags) {
    d->edit_flags = flags;
    Q_EMIT editFlagsChanged();
  }
}

const QString& OxideQQuickTouchSelectionController::selectedText() const {
  Q_D(const OxideQQuickTouchSelectionController);

  return d->selected_text;
}

void OxideQQuickTouchSelectionController::setSelectedText(
    const QString& selectedText) {
  Q_D(OxideQQuickTouchSelectionController);

  if (selectedText != d->selected_text) {
    d->selected_text = selectedText;
    Q_EMIT selectedTextChanged();
  }
}
