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

#include "base/logging.h"

#include "javascript_dialog.h"
#include "javascript_dialog_contents_helper.h"
#include "javascript_dialog_factory.h"

namespace oxide {

void JavaScriptDialogHost::Close(bool success,
                                 const base::string16& user_input) {
  if (callback_.is_null()) {
    return;
  }

  callback_.Run(success, user_input);
  callback_.Reset();
  dialog_->Hide();
}

JavaScriptDialogHost::JavaScriptDialogHost(
    base::WeakPtr<JavaScriptDialogContentsHelper> owner,
    const GURL& origin_url,
    bool is_before_unload_dialog,
    content::JavaScriptDialogType type,
    const base::string16& message_text,
    const base::string16& default_prompt_text,
    const content::JavaScriptDialogManager::DialogClosedCallback& callback)
    : owner_(owner),
      origin_url_(origin_url),
      is_before_unload_dialog_(is_before_unload_dialog),
      type_(type),
      message_text_(message_text),
      default_prompt_text_(default_prompt_text),
      callback_(callback) {}

JavaScriptDialogHost::~JavaScriptDialogHost() = default;

bool JavaScriptDialogHost::Show() {
  if (!owner_->factory()) {
    callback_.Run(is_before_unload_dialog_, base::string16());
    return false;
  }

  if (is_before_unload_dialog_) {
    dialog_ = owner_->factory()->CreateBeforeUnloadDialog(this, origin_url_);
  } else {
    dialog_ = owner_->factory()->CreateJavaScriptDialog(this,
                                                        origin_url_,
                                                        type_,
                                                        message_text_,
                                                        default_prompt_text_);
  }

  if (!dialog_) {
    callback_.Run(is_before_unload_dialog_, base::string16());
    return false;
  }

  dialog_->Show();

  return true;
}

void JavaScriptDialogHost::Dismiss() {
  DCHECK(!callback_.is_null());

  callback_.Run(false, base::string16());
  callback_.Reset();
  dialog_->Hide();
}

void JavaScriptDialogHost::Handle(bool success,
                                  const base::string16* prompt_override) {
  DCHECK(!callback_.is_null());

  callback_.Run(success,
                prompt_override ?
                    *prompt_override
                    : dialog_->GetCurrentPromptText());
  callback_.Reset();
  dialog_->Hide();
}

} // namespace oxide
