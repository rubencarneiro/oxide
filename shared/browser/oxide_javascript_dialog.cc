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

#include "oxide_javascript_dialog.h"

#include "shared/browser/oxide_javascript_dialog_manager.h"

namespace oxide {

JavaScriptDialog::JavaScriptDialog() :
    is_reload_(false),
    is_before_unload_dialog_(false) {}

JavaScriptDialog::~JavaScriptDialog() {}

void JavaScriptDialog::Close(bool accept, const base::string16& user_input) {
  callback_.Run(accept, user_input);
  JavaScriptDialogManager::GetInstance()->OnDialogClosed(web_contents_, this);
}

void JavaScriptDialog::CouldNotShow() {
  Close(is_before_unload_dialog_);
}

void JavaScriptDialog::Cancel() {
  callback_.Run(false, base::string16());
  JavaScriptDialogManager::GetInstance()->OnDialogCancelled(web_contents_, this);
}

} // namespace oxide
