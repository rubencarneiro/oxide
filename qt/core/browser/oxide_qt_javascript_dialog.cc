// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#include "oxide_qt_javascript_dialog.h"

#include "base/strings/utf_string_conversions.h"

#include "qt/core/glue/oxide_qt_javascript_dialog_proxy.h"

namespace oxide {
namespace qt {

JavaScriptDialog::~JavaScriptDialog() {}

bool JavaScriptDialog::Run() {
  return proxy_->Show();
}

void JavaScriptDialog::Hide() {
  proxy_->Hide();
}

void JavaScriptDialog::Handle(bool accept,
                              const base::string16* prompt_override) {
  QString prompt;
  if (prompt_override) {
    prompt = QString::fromStdString(base::UTF16ToUTF8(*prompt_override));
  }

  proxy_->Handle(accept, prompt);
}

void JavaScriptDialog::close(bool accept, const QString& user_input) {
  Close(accept, base::UTF8ToUTF16(user_input.toStdString()));
}

QUrl JavaScriptDialog::originUrl() const {
  return QUrl(QString::fromStdString(origin_url_.spec()));
}

QString JavaScriptDialog::messageText() const {
  return QString::fromStdString(base::UTF16ToUTF8(message_text_));
}

QString JavaScriptDialog::defaultPromptText() const {
  return QString::fromStdString(base::UTF16ToUTF8(default_prompt_text_));
}

JavaScriptDialog::JavaScriptDialog() {}

void JavaScriptDialog::SetProxy(JavaScriptDialogProxy* proxy) {
  proxy_.reset(proxy);
}

} // namespace qt
} // namespace oxide
