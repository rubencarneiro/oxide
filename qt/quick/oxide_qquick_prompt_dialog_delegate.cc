// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2014 Canonical Ltd.

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

#include "oxide_qquick_prompt_dialog_delegate.h"

#include <QObject>
#include <QString>

#include "qt/quick/api/oxideqquickwebview_p.h"

namespace oxide {
namespace qquick {

class PromptDialogContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString message READ message CONSTANT FINAL)
  Q_PROPERTY(QString defaultValue READ defaultValue CONSTANT FINAL)
  Q_PROPERTY(QString currentValue READ currentValue WRITE setCurrentValue NOTIFY currentValueChanged)

 public:
  virtual ~PromptDialogContext() {}
  PromptDialogContext(PromptDialogDelegate* delegate);

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
  PromptDialogDelegate* delegate_;
  QString currentValue_;
};

PromptDialogContext::PromptDialogContext(PromptDialogDelegate* delegate) :
    delegate_(delegate) {}

QString PromptDialogContext::message() const {
  return delegate_->messageText();
}

QString PromptDialogContext::defaultValue() const {
  return delegate_->defaultPromptText();
}

const QString& PromptDialogContext::currentValue() const {
  return currentValue_;
}

void PromptDialogContext::setCurrentValue(const QString& value) {
  if (value != currentValue_) {
    currentValue_ = value;
    Q_EMIT currentValueChanged();
  }
}

void PromptDialogContext::accept(const QString& value) const {
  delegate_->Close(true, value);
}

void PromptDialogContext::reject() const {
  delegate_->Close(false);
}

PromptDialogDelegate::PromptDialogDelegate(OxideQQuickWebView* webview) :
    JavaScriptDialogDelegate(webview) {}

bool PromptDialogDelegate::Show() {
  return show(new PromptDialogContext(this), web_view_->promptDialog());
}

void PromptDialogDelegate::Handle(bool accept, const QString& prompt_override) {
  PromptDialogContext* contextObject =
    qobject_cast<PromptDialogContext*>(context_->contextObject());
  if (accept) {
    if (prompt_override.isNull()) {
      contextObject->accept(contextObject->currentValue());
    } else {
      contextObject->accept(prompt_override);
    }
  } else {
    contextObject->reject();
  } 
}

} // namespace qquick
} // namespace oxide

#include "oxide_qquick_prompt_dialog_delegate.moc"
