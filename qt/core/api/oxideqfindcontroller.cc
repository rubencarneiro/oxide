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

#include "oxideqfindcontroller.h"
#include "oxideqfindcontroller_p.h"

#include <QtDebug>

#include "content/browser/web_contents/web_contents_impl.h"
#include "qt/core/browser/oxide_qt_web_view.h"
#include "shared/browser/oxide_web_view.h"

OxideQFindControllerPrivate::OxideQFindControllerPrivate()
    : view(nullptr),
      case_sensitive_(false),
      count_(0),
      current_(0),
      request_id_(0) {}

OxideQFindControllerPrivate::~OxideQFindControllerPrivate() {}

// static
OxideQFindControllerPrivate* OxideQFindControllerPrivate::get(
    OxideQFindController* q) {
  return q->d_func();
}

OxideQFindController::OxideQFindController()
    : d_ptr(new OxideQFindControllerPrivate()) {}

OxideQFindController::~OxideQFindController() {}

QString OxideQFindController::text() const {
  Q_D(const OxideQFindController);

  if (!d->view) {
    return QString();
  }

  return QString::fromStdString(d->view->GetFindInPageText());
}

void OxideQFindController::setText(const QString& newText) {
  Q_D(OxideQFindController);

  if (!d->view) {
    // FIXME(chrisccoulson)
    qWarning() <<
        "OxideQFindController::setText: Cannot use FindController during "
        "webview construction";
    return;
  }

  if (newText == text()) {
    return;
  }

  d->view->SetFindInPageText(newText.toStdString());
  emit textChanged();
}

bool OxideQFindController::caseSensitive() const {
  Q_D(const OxideQFindController);

  if (!d->view) {
    return false;
  }

  return d->view->GetFindInPageCaseSensitive();
}

void OxideQFindController::setCaseSensitive(bool newCaseSensitive) {
  Q_D(OxideQFindController);

  if (!d->view) {
    // FIXME(chrisccoulson)
    qWarning() <<
        "OxideQFindController::setCaseSensitive: Cannot use FindController "
        "during webview construction";
    return;
  }

  if (newCaseSensitive == caseSensitive()) {
      return;
  }

  d->view->SetFindInPageCaseSensitive(newCaseSensitive);
  emit caseSensitiveChanged();
}

int OxideQFindController::count() const {
  Q_D(const OxideQFindController);

  if (!d->view) {
    return 0;
  }

  return d->view->GetFindInPageCount();
}

int OxideQFindController::current() const {
  Q_D(const OxideQFindController);

  if (!d->view) {
    return 0;
  }

  return d->view->GetFindInPageCurrent();
}

void OxideQFindController::next() {
  Q_D(OxideQFindController);

  if (!d->view) {
    return;
  }

  d->view->FindInPageNext();
}

void OxideQFindController::previous() {
  Q_D(OxideQFindController);

  if (!d->view) {
    return;
  }

  d->view->FindInPagePrevious();
}
