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

#include "oxide_qquick_prompt_dialog_delegate.h"

#include <QObject>
#include <QString>

#include "qt/core/glue/oxide_qt_javascript_dialog_closed_callback.h"

namespace oxide {
namespace qquick {

class PromptDialogContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString message READ message CONSTANT FINAL)
  Q_PROPERTY(QString defaultValue READ defaultValue CONSTANT FINAL)

 public:
  virtual ~PromptDialogContext() {}
  PromptDialogContext(OxideQQuickPromptDialogDelegate* delegate,
                      const QString& message,
                      const QString& default_value,
                      oxide::qt::JavaScriptDialogClosedCallback* callback);

  const QString& message() const { return message_; }
  const QString& defaultValue() const { return defaultValue_; }

 public Q_SLOTS:
  void accept(const QString& value) const;
  void reject() const;

 private:
  OxideQQuickPromptDialogDelegate* delegate_;
  QString message_;
  QString defaultValue_;
  oxide::qt::JavaScriptDialogClosedCallback* callback_;
};

PromptDialogContext::PromptDialogContext(
    OxideQQuickPromptDialogDelegate* delegate,
    const QString& message,
    const QString& default_value,
    oxide::qt::JavaScriptDialogClosedCallback* callback) :
    delegate_(delegate),
    message_(message),
    defaultValue_(default_value),
    callback_(callback) {}

void PromptDialogContext::accept(const QString& value) const {
  delegate_->Hide();
  callback_->run(true, value);
}

void PromptDialogContext::reject() const {
  delegate_->Hide();
  callback_->run(false);
}

OxideQQuickPromptDialogDelegate::OxideQQuickPromptDialogDelegate(
    OxideQQuickWebView* webview) :
    OxideQQuickJavaScriptDialogDelegate(webview) {}

void OxideQQuickPromptDialogDelegate::Show(
    const QUrl& origin_url,
    const QString& accept_lang,
    const QString& message_text,
    const QString& default_prompt_text,
    oxide::qt::JavaScriptDialogClosedCallback* callback,
    bool* did_suppress_message) {
  Q_UNUSED(origin_url);
  Q_UNUSED(accept_lang);

  *did_suppress_message = false;

  PromptDialogContext* contextObject = new PromptDialogContext(this, message_text, default_prompt_text, callback);
  show(contextObject, callback);
}

} // namespace qquick
} // namespace oxide

#include "oxide_qquick_prompt_dialog_delegate.moc"
