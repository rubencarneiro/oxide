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

#include "oxide_qt_javascript_dialog_delegate.h"

#include "base/strings/utf_string_conversions.h"

#include "qt/core/browser/oxide_qt_javascript_dialog.h"

namespace oxide {
namespace qt {

JavaScriptDialogDelegate::JavaScriptDialogDelegate() :
    dialog_(nullptr) {}

JavaScriptDialogDelegate::~JavaScriptDialogDelegate() {}

void JavaScriptDialogDelegate::Close(bool accept, const QString& user_input) {
  dialog_->Close(accept, base::UTF8ToUTF16(user_input.toStdString()));
}

void JavaScriptDialogDelegate::Handle(bool accept,
                                      const QString& prompt_override) {
  // default implementation, overridden in PromptDialogDelegate
  Q_UNUSED(prompt_override);
  Close(accept);
}

QUrl JavaScriptDialogDelegate::originUrl() const {
  return dialog_->originUrl();
}

QString JavaScriptDialogDelegate::acceptLang() const {
  return dialog_->acceptLang();
}

QString JavaScriptDialogDelegate::messageText() const {
  return dialog_->messageText();
}

QString JavaScriptDialogDelegate::defaultPromptText() const {
  return dialog_->defaultPromptText();
}

} // namespace qt
} // namespace oxide
