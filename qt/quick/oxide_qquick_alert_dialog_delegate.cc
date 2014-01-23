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

#include "oxide_qquick_alert_dialog_delegate.h"

#include <QObject>
#include <QString>

#include "qt/core/glue/oxide_qt_javascript_dialog_closed_callback.h"

namespace oxide {
namespace qquick {

class AlertDialogContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString message READ message CONSTANT FINAL)

 public:
  virtual ~AlertDialogContext() {}
  AlertDialogContext(OxideQQuickAlertDialogDelegate* delegate,
                     const QString& message,
                     oxide::qt::JavaScriptDialogClosedCallback* callback);

  const QString& message() const { return message_; }

 public Q_SLOTS:
  void accept() const;

 private:
  OxideQQuickAlertDialogDelegate* delegate_;
  QString message_;
  oxide::qt::JavaScriptDialogClosedCallback* callback_;
};

AlertDialogContext::AlertDialogContext(
    OxideQQuickAlertDialogDelegate* delegate,
    const QString& message,
    oxide::qt::JavaScriptDialogClosedCallback* callback) :
    delegate_(delegate),
    message_(message),
    callback_(callback) {}

void AlertDialogContext::accept() const {
  callback_->run(true);
  delegate_->deleteLater();
}

OxideQQuickAlertDialogDelegate::OxideQQuickAlertDialogDelegate(
    OxideQQuickWebView* webview,
    QQmlComponent* component) :
    OxideQQuickJavaScriptDialogDelegate(webview, component) {}

bool OxideQQuickAlertDialogDelegate::Show(
    const QUrl& origin_url,
    const QString& accept_lang,
    const QString& message_text,
    oxide::qt::JavaScriptDialogClosedCallback* callback,
    bool* did_suppress_message) {
  Q_UNUSED(origin_url);
  Q_UNUSED(accept_lang);

  *did_suppress_message = false;
  return show(new AlertDialogContext(this, message_text, callback));
}

bool OxideQQuickAlertDialogDelegate::Handle(
    bool accept,
    const QString& prompt_override) {
  Q_UNUSED(accept);
  Q_UNUSED(prompt_override);

  if (isShown()) {
    AlertDialogContext* contextObject =
        qobject_cast<AlertDialogContext*>(context_->contextObject());
    contextObject->accept();
    return true;
  } else {
    return false;
  }
}

} // namespace qquick
} // namespace oxide

#include "oxide_qquick_alert_dialog_delegate.moc"
