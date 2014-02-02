// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include "qt/core/glue/oxide_qt_javascript_dialog_delegate.h"

namespace oxide {
namespace qt {

JavaScriptDialog::JavaScriptDialog(
    JavaScriptDialogDelegate* delegate,
    bool* did_suppress_message) :
    delegate_(delegate) {
  delegate_->SetDialog(this);
}

void JavaScriptDialog::Run() {
  if (!delegate_->Show()) {
    CouldNotShow();
  }
}

void JavaScriptDialog::Handle(bool accept,
                              const base::string16* prompt_override) {
  QString prompt;
  if (prompt_override) {
    prompt = QString::fromStdString(base::UTF16ToUTF8(*prompt_override));
  }
  delegate_->Handle(accept, prompt);
}

QUrl JavaScriptDialog::originUrl() const {
  return QUrl(QString::fromStdString(origin_url_.spec()));
}

QString JavaScriptDialog::acceptLang() const {
  return QString::fromStdString(accept_lang_);
}

QString JavaScriptDialog::messageText() const {
  return QString::fromStdString(base::UTF16ToUTF8(message_text_));
}

QString JavaScriptDialog::defaultPromptText() const {
  return QString::fromStdString(base::UTF16ToUTF8(default_prompt_text_));
}

} // namespace qt
} // namespace oxide
