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

#include "content/browser/web_contents/web_contents_impl.h"
#include "qt/core/browser/oxide_qt_web_view.h"
#include "shared/browser/oxide_web_view.h"

OxideQFindControllerPrivate::OxideQFindControllerPrivate(oxide::WebView* webview) :
    case_sensitive_(false),
    count_(0),
    current_(0),
    request_id_(0),
    webview_(webview) {}

OxideQFindControllerPrivate::~OxideQFindControllerPrivate() {}

OxideQFindController::OxideQFindController(oxide::WebView* webview) :
    QObject(nullptr),
    d_ptr(new OxideQFindControllerPrivate(webview)) {
}

OxideQFindController::~OxideQFindController() {}

QString OxideQFindController::text() const {
  Q_D(const OxideQFindController);

  return QString::fromStdString(d->webview_->GetFindInPageText());
}

void OxideQFindController::setText(const QString& newText) {
  Q_D(OxideQFindController);

  if (newText == text()) {
    return;
  }

  d->webview_->SetFindInPageText(newText.toStdString());
  emit textChanged();
}

bool OxideQFindController::caseSensitive() const {
  Q_D(const OxideQFindController);

  return d->webview_->GetFindInPageCaseSensitive();
}

void OxideQFindController::setCaseSensitive(bool newCaseSensitive) {
  Q_D(OxideQFindController);

  if (newCaseSensitive == caseSensitive()) {
      return;
  }

  d->webview_->SetFindInPageCaseSensitive(newCaseSensitive);
  emit caseSensitiveChanged();
}

int OxideQFindController::count() const {
  Q_D(const OxideQFindController);

  return d->webview_->GetFindInPageCount();
}

int OxideQFindController::current() const {
  Q_D(const OxideQFindController);

  return d->webview_->GetFindInPageCurrent();
}

void OxideQFindController::next() {
  Q_D(OxideQFindController);

  d->webview_->FindInPageNext();
}

void OxideQFindController::previous() {
  Q_D(OxideQFindController);

  d->webview_->FindInPagePrevious();
}
