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

#include "qquick_legacy_confirm_dialog.h"

#include <QObject>

#include "qt/core/glue/javascript_dialog_client.h"

namespace oxide {
namespace qquick {

using qt::JavaScriptDialogClient;

class ConfirmDialogContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString message READ message CONSTANT FINAL)

 public:
  ~ConfirmDialogContext() override {}
  ConfirmDialogContext(JavaScriptDialogClient* client,
                       const QString& message_text);

  QString message() const;

 public Q_SLOTS:
  void accept() const;
  void reject() const;

 private:
  JavaScriptDialogClient* client_;
  QString message_text_;
};

ConfirmDialogContext::ConfirmDialogContext(JavaScriptDialogClient* client,
                                           const QString& message_text)
    : client_(client),
      message_text_(message_text) {}

QString ConfirmDialogContext::message() const {
  return message_text_;
}

void ConfirmDialogContext::accept() const {
  client_->close(true);
}

void ConfirmDialogContext::reject() const {
  client_->close(false);
}

void LegacyConfirmDialog::Show() {
  run(new ConfirmDialogContext(client_, message_text_));
}

LegacyConfirmDialog::LegacyConfirmDialog(QQuickItem* parent,
                                         QQmlComponent* component,
                                         const QString& message_text,
                                         JavaScriptDialogClient* client)
    : LegacyJavaScriptDialog(parent, component, client),
      message_text_(message_text) {}

} // namespace qquick
} // namespace oxide

#include "qquick_legacy_confirm_dialog.moc"
