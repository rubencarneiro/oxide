// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#include "oxide_qquick_before_unload_dialog_delegate.h"

#include <QObject>

#include "qt/quick/api/oxideqquickwebview_p.h"

namespace oxide {
namespace qquick {

class BeforeUnloadDialogContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString message READ message CONSTANT FINAL)

 public:
  virtual ~BeforeUnloadDialogContext() {}
  BeforeUnloadDialogContext(BeforeUnloadDialogDelegate* delegate);

  QString message() const;

 public Q_SLOTS:
  void accept() const;
  void reject() const;

 private:
  BeforeUnloadDialogDelegate* delegate_;
};

BeforeUnloadDialogContext::BeforeUnloadDialogContext(
    BeforeUnloadDialogDelegate* delegate) :
    delegate_(delegate) {}

QString BeforeUnloadDialogContext::message() const {
  return delegate_->messageText();
}

void BeforeUnloadDialogContext::accept() const {
  delegate_->Close(true);
}

void BeforeUnloadDialogContext::reject() const {
  delegate_->Close(false);
}

BeforeUnloadDialogDelegate::BeforeUnloadDialogDelegate(
    OxideQQuickWebView* webview) :
    JavaScriptDialogDelegate(webview) {}

bool BeforeUnloadDialogDelegate::Show() {
  return show(new BeforeUnloadDialogContext(this),
              web_view_->beforeUnloadDialog());
}

} // namespace qquick
} // namespace oxide

#include "oxide_qquick_before_unload_dialog_delegate.moc"
