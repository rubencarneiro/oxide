// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2016 Canonical Ltd.

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

#include "javascript_dialog_host.h"

#include <string>

#include <QString>

#include "base/strings/utf_string_conversions.h"

#include "qt/core/glue/javascript_dialog.h"
#include "shared/browser/javascript_dialogs/javascript_dialog_client.h"

namespace oxide {
namespace qt {

void JavaScriptDialogHost::Show() {
  dialog_->Show();
}

void JavaScriptDialogHost::Hide() {
  dialog_->Hide();
}

base::string16 JavaScriptDialogHost::GetCurrentPromptText() {
  return base::UTF8ToUTF16(dialog_->GetCurrentPromptText().toStdString());
}

void JavaScriptDialogHost::close(bool accept, const QString& user_input) {
  client_->Close(accept, base::UTF8ToUTF16(user_input.toStdString()));
}

JavaScriptDialogHost::JavaScriptDialogHost(
    oxide::JavaScriptDialogClient* client)
    : client_(client) {}

JavaScriptDialogHost::~JavaScriptDialogHost() = default;

void JavaScriptDialogHost::Init(std::unique_ptr<qt::JavaScriptDialog> dialog) {
  dialog_ = std::move(dialog);
}

} // namespace qt
} // namespace oxide
