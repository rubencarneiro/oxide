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

#include "qt/core/browser/oxide_qt_find_controller.h"
#include "shared/browser/oxide_find_controller.h"

OxideQFindControllerPrivate::OxideQFindControllerPrivate(
    OxideQFindController* q)
    : q_ptr(q),
      controller_(new oxide::qt::FindController(q)),
      case_sensitive_(false) {}

OxideQFindControllerPrivate::~OxideQFindControllerPrivate() {}

// static
OxideQFindController* OxideQFindControllerPrivate::Create() {
  return new OxideQFindController();
}

// static
OxideQFindControllerPrivate* OxideQFindControllerPrivate::get(
    OxideQFindController* q) {
  return q->d_func();
}

/*!
\class OxideQFindController
\inheaderfile oxideqfindcontroller.h
\inmodule OxideQtCore

\brief Find-in-page helper

OxideQFindController provides a mechanism to allow an application to provide
find-in-page functionality.

Applications specify the search term by setting \l{text}. In response, the
engine highlights any matches and updates \l{count} with the number of results.
The application can then cycle through the results by calls to \l{previous} and
\l{next}. The current position is indicated by \l{current}.

The current find-in-page request can be terminated by setting \l{text} to an
empty string.
*/

OxideQFindController::OxideQFindController()
    : d_ptr(new OxideQFindControllerPrivate(this)) {}

/*!
\internal
*/

OxideQFindController::~OxideQFindController() {}

/*!
\property OxideQFindController::text

Set this to the desired search term to begin a search in the current page. Set
it to an empty string to terminate the current search.
*/

QString OxideQFindController::text() const {
  Q_D(const OxideQFindController);

  return d->text_;
}

void OxideQFindController::setText(const QString& newText) {
  Q_D(OxideQFindController);

  if (!d->controller_->IsInitialized()) {
    // FIXME(chrisccoulson)
    qWarning() <<
        "OxideQFindController::setText: Cannot use FindController during "
        "webview construction";
    return;
  }

  if (newText == d->text_) {
    return;
  }

  d->text_ = newText;
  emit textChanged();

  d->controller_->StartFinding(d->text_.toStdString(), d->case_sensitive_);
}

/*!
\property OxideQFindController::caseSensitive

Set this to true if the search should be case sensitive.
*/

bool OxideQFindController::caseSensitive() const {
  Q_D(const OxideQFindController);

  return d->case_sensitive_;
}

void OxideQFindController::setCaseSensitive(bool newCaseSensitive) {
  Q_D(OxideQFindController);

  if (!d->controller_->IsInitialized()) {
    // FIXME(chrisccoulson)
    qWarning() <<
        "OxideQFindController::setCaseSensitive: Cannot use FindController "
        "during webview construction";
    return;
  }

  if (newCaseSensitive == d->case_sensitive_) {
      return;
  }

  d->case_sensitive_ = newCaseSensitive;
  emit caseSensitiveChanged();

  d->controller_->StartFinding(d->text_.toStdString(), d->case_sensitive_);
}

/*!
\property OxideQFindController::count

The number of results found. This will be 0 if \l{text} is an empty string.
*/

int OxideQFindController::count() const {
  Q_D(const OxideQFindController);

  return d->controller_->GetResult().number_of_matches;
}

/*!
\property OxideQFindController::current

The current position within the search results. This will be a number between 0
and \l{count}, and will be 0 if \l{text} is an empty string.
*/

int OxideQFindController::current() const {
  Q_D(const OxideQFindController);

  return d->controller_->GetResult().active_match_ordinal;
}

/*!
Advance the document to the next search result. This will result in \l{current}
being incremented by 1.
*/

void OxideQFindController::next() {
  Q_D(OxideQFindController);

  if (!d->controller_->IsRequestActive()) {
    qWarning() <<
        "OxideQFindController::next : There is currently no active "
        "find-in-page request";
    return;
  }

  d->controller_->GotoNextMatch();
}

/*!
Move the document back to the previous search result. This will result in
\l{current} being decremented by 1.
*/

void OxideQFindController::previous() {
  Q_D(OxideQFindController);

  if (!d->controller_->IsRequestActive()) {
    qWarning() <<
        "OxideQFindController::previous : There is currently no active "
        "find-in-page request";
    return;
  }

  d->controller_->GotoPreviousMatch();
}
