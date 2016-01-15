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

#include "oxide_qquick_before_unload_dialog.h"

#include <QObject>

#include "qt/core/glue/oxide_qt_javascript_dialog_proxy_client.h"
#include "qt/quick/api/oxideqquickwebview.h"

namespace oxide {
namespace qquick {

class BeforeUnloadDialogContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString message READ message CONSTANT FINAL)

 public:
  virtual ~BeforeUnloadDialogContext() {}
  BeforeUnloadDialogContext(oxide::qt::JavaScriptDialogProxyClient* client);

  QString message() const;

 public Q_SLOTS:
  void accept() const;
  void reject() const;

 private:
  oxide::qt::JavaScriptDialogProxyClient* client_;
};

BeforeUnloadDialogContext::BeforeUnloadDialogContext(
    oxide::qt::JavaScriptDialogProxyClient* client)
    : client_(client) {}

QString BeforeUnloadDialogContext::message() const {
  return client_->messageText();
}

void BeforeUnloadDialogContext::accept() const {
  client_->close(true);
}

void BeforeUnloadDialogContext::reject() const {
  client_->close(false);
}

bool BeforeUnloadDialog::Show() {
  if (!view_) {
    qWarning() << "BeforeUnloadDialog::Show: Can't show after the view has gone";
    return false;
  }

  return run(new BeforeUnloadDialogContext(client_),
             view_->beforeUnloadDialog());
}

BeforeUnloadDialog::BeforeUnloadDialog(
    OxideQQuickWebView* view,
    oxide::qt::JavaScriptDialogProxyClient* client)
    : JavaScriptDialog(view, client) {}

} // namespace qquick
} // namespace oxide

#include "oxide_qquick_before_unload_dialog.moc"
