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
#include "third_party/WebKit/public/web/WebFindOptions.h"

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

const QString& OxideQFindController::text() const {
  Q_D(const OxideQFindController);

  return d->text_;
}

void OxideQFindController::setText(const QString& text) {
  Q_D(OxideQFindController);

  if (d->text_ == text) {
    return;
  }

  d->text_ = text;
  emit textChanged();

  updateOnParametersChanged();
}

bool OxideQFindController::caseSensitive() const {
  Q_D(const OxideQFindController);

  return d->case_sensitive_;
}

void OxideQFindController::setCaseSensitive(bool caseSensitive) {
  Q_D(OxideQFindController);

  if (d->case_sensitive_ == caseSensitive) {
      return;
  }

  d->case_sensitive_ = caseSensitive;
  emit caseSensitiveChanged();

  updateOnParametersChanged();
}

void OxideQFindController::updateOnParametersChanged() {
  Q_D(OxideQFindController);

  content::WebContents* contents = d->webview_->GetWebContents();
  if (!contents) {
    return;
  }

  contents->StopFinding(content::STOP_FIND_ACTION_CLEAR_SELECTION);
  d->current_ = 0;
  emit currentChanged();
  d->count_ = 0;
  emit countChanged();

  if (!d->text_.isEmpty()) {
    d->request_id_++;

    blink::WebFindOptions options;
    options.forward = true;
    options.findNext = false;
    options.matchCase = d->case_sensitive_;

    contents->Find(d->request_id_, d->text_.utf16(), options);
  }
}

void OxideQFindController::updateOnFindResult(int current, int count) {
  Q_D(OxideQFindController);

  if (count != -1 && d->count_ != count) {
    d->count_ = count;
    Q_EMIT countChanged();
  }

  if (current != -1 && d->current_ != current) {
    d->current_ = current;
    Q_EMIT currentChanged();
  }
}

int OxideQFindController::count() const {
  Q_D(const OxideQFindController);

  return d->count_;
}

int OxideQFindController::current() const {
  Q_D(const OxideQFindController);

  return d->current_;
}

void OxideQFindController::next() {
   Q_D(OxideQFindController);

   content::WebContents* contents = d->webview_->GetWebContents();
   if (!contents) {
     return;
   }

   blink::WebFindOptions options;
   options.forward = true;
   options.findNext = true;

   contents->Find(d->request_id_, d->text_.utf16(), options);
}

void OxideQFindController::previous() {
   Q_D(OxideQFindController);

  content::WebContents* contents = d->webview_->GetWebContents();
  if (!contents) {
    return;
  }

  blink::WebFindOptions options;
  options.forward = false;
  options.findNext = true;

  contents->Find(d->request_id_, d->text_.utf16(), options);
}
