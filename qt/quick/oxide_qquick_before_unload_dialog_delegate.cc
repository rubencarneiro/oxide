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
#include <QString>

#include "qt/core/glue/oxide_qt_javascript_dialog_closed_callback.h"

namespace oxide {
namespace qquick {

class BeforeUnloadDialogContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString message READ message CONSTANT FINAL)

 public:
  virtual ~BeforeUnloadDialogContext() {}
  BeforeUnloadDialogContext(
      OxideQQuickBeforeUnloadDialogDelegate* delegate,
      const QString& message,
      oxide::qt::JavaScriptDialogClosedCallback* callback);

  const QString& message() const { return message_; }

 public Q_SLOTS:
  void accept() const;
  void reject() const;

 private:
  OxideQQuickBeforeUnloadDialogDelegate* delegate_;
  QString message_;
  oxide::qt::JavaScriptDialogClosedCallback* callback_;
};

BeforeUnloadDialogContext::BeforeUnloadDialogContext(
    OxideQQuickBeforeUnloadDialogDelegate* delegate,
    const QString& message,
    oxide::qt::JavaScriptDialogClosedCallback* callback) :
    delegate_(delegate),
    message_(message),
    callback_(callback) {}

void BeforeUnloadDialogContext::accept() const {
  callback_->run(true);
  delegate_->deleteLater();
}

void BeforeUnloadDialogContext::reject() const {
  callback_->run(false);
  delegate_->deleteLater();
}

OxideQQuickBeforeUnloadDialogDelegate::OxideQQuickBeforeUnloadDialogDelegate(
    OxideQQuickWebView* webview,
    QQmlComponent* component) :
    OxideQQuickJavaScriptDialogDelegate(webview, component) {}

bool OxideQQuickBeforeUnloadDialogDelegate::Show(
    const QString& message_text,
    bool is_reload,
    oxide::qt::JavaScriptDialogClosedCallback* callback) {
  Q_UNUSED(is_reload);

  return show(new BeforeUnloadDialogContext(this, message_text, callback));
}

bool OxideQQuickBeforeUnloadDialogDelegate::Handle(
    bool accept,
    const QString& prompt_override) {
  Q_UNUSED(prompt_override);

  if (isShown()) {
    BeforeUnloadDialogContext* contextObject =
        qobject_cast<BeforeUnloadDialogContext*>(context_->contextObject());
    if (accept) {
      contextObject->accept();
    } else {
      contextObject->reject();
    }
    return true;
  } else {
    return false;
  }
}

} // namespace qquick
} // namespace oxide

#include "oxide_qquick_before_unload_dialog_delegate.moc"
