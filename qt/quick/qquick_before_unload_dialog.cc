// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include "qquick_before_unload_dialog.h"

#include <libintl.h>
#include <QObject>

#include "qt/core/api/oxideqglobal_p.h"
#include "qt/core/glue/javascript_dialog_client.h"

namespace oxide {
namespace qquick {

using qt::JavaScriptDialogClient;

class BeforeUnloadDialogContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString message READ message CONSTANT FINAL)

 public:
  ~BeforeUnloadDialogContext() override {}
  BeforeUnloadDialogContext(JavaScriptDialogClient* client);

  QString message() const;

 public Q_SLOTS:
  void accept() const;
  void reject() const;

 private:
  JavaScriptDialogClient* client_;
};

BeforeUnloadDialogContext::BeforeUnloadDialogContext(
    JavaScriptDialogClient* client)
    : client_(client) {}

QString BeforeUnloadDialogContext::message() const {
  WARN_DEPRECATED_API_USAGE() <<
      "BeforeUnloadDialogContext::message is deprecated and the message text "
      "provided by the web page is ignored. This API returns a message for "
      "compatibility purposes, but applications should stop using it";
  return QString(
      dgettext(OXIDE_GETTEXT_DOMAIN, "Changes you made may not be saved."));
}

void BeforeUnloadDialogContext::accept() const {
  client_->close(true);
}

void BeforeUnloadDialogContext::reject() const {
  client_->close(false);
}

bool BeforeUnloadDialog::Show() {
  return run(new BeforeUnloadDialogContext(client_));
}

BeforeUnloadDialog::BeforeUnloadDialog(QQuickItem* parent,
                                       QQmlComponent* component,
                                       JavaScriptDialogClient* client)
    : JavaScriptDialog(parent, component, client) {}

} // namespace qquick
} // namespace oxide

#include "qquick_before_unload_dialog.moc"
