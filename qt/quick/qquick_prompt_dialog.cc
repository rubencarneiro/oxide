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

#include "qquick_prompt_dialog.h"

#include <QObject>

#include "qt/core/glue/javascript_dialog_client.h"

namespace oxide {
namespace qquick {

using qt::JavaScriptDialogClient;

class PromptDialogContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString message READ message CONSTANT FINAL)
  Q_PROPERTY(QString defaultValue READ defaultValue CONSTANT FINAL)
  Q_PROPERTY(QString currentValue READ currentValue WRITE setCurrentValue NOTIFY currentValueChanged)

 public:
  ~PromptDialogContext() override {}
  PromptDialogContext(JavaScriptDialogClient* client,
                      const QString& message_text,
                      const QString& default_prompt_text);

  QString message() const;
  QString defaultValue() const;
  const QString& currentValue() const;
  void setCurrentValue(const QString& value);

 Q_SIGNALS:
  void currentValueChanged() const;

 public Q_SLOTS:
  void accept(const QString& value) const;
  void reject() const;

 private:
  JavaScriptDialogClient* client_;
  QString message_text_;
  QString default_prompt_text_;

  QString current_value_;
};

PromptDialogContext::PromptDialogContext(JavaScriptDialogClient* client,
                                         const QString& message_text,
                                         const QString& default_prompt_text)
    : client_(client),
      message_text_(message_text),
      default_prompt_text_(default_prompt_text) {}

QString PromptDialogContext::message() const {
  return message_text_;
}

QString PromptDialogContext::defaultValue() const {
  return default_prompt_text_;
}

const QString& PromptDialogContext::currentValue() const {
  return current_value_;
}

void PromptDialogContext::setCurrentValue(const QString& value) {
  if (value != current_value_) {
    current_value_ = value;
    Q_EMIT currentValueChanged();
  }
}

void PromptDialogContext::accept(const QString& value) const {
  client_->close(true, value);
}

void PromptDialogContext::reject() const {
  client_->close(false);
}

void PromptDialog::Show() {
  run(new PromptDialogContext(client_, message_text_, default_prompt_text_));
}

QString PromptDialog::GetCurrentPromptText() {
  PromptDialogContext* context_object =
      qobject_cast<PromptDialogContext*>(context_->contextObject());
  return context_object->currentValue();
}

PromptDialog::PromptDialog(QQuickItem* parent,
                           QQmlComponent* component,
                           const QString& message_text,
                           const QString& default_prompt_text,
                           JavaScriptDialogClient* client)
    : JavaScriptDialog(parent, component, client),
      message_text_(message_text),
      default_prompt_text_(default_prompt_text) {}

} // namespace qquick
} // namespace oxide

#include "qquick_prompt_dialog.moc"
