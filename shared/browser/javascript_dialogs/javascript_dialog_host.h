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

#ifndef _OXIDE_SHARED_BROWSER_JAVASCRIPT_DIALOGS_JAVASCRIPT_DIALOG_HOST_H_
#define _OXIDE_SHARED_BROWSER_JAVASCRIPT_DIALOGS_JAVASCRIPT_DIALOG_HOST_H_

#include <memory>

#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/strings/string16.h"
#include "content/public/browser/javascript_dialog_manager.h"
#include "content/public/common/javascript_message_type.h"

#include "shared/browser/javascript_dialogs/javascript_dialog_client.h"
#include "shared/common/oxide_shared_export.h"

namespace oxide {

class JavaScriptDialog;
class JavaScriptDialogContentsHelper;

class OXIDE_SHARED_EXPORT JavaScriptDialogHost : public JavaScriptDialogClient {
 public:
  JavaScriptDialogHost(
      JavaScriptDialogContentsHelper* owner,
      const GURL& origin_url,
      bool is_before_unload_dialog,
      content::JavaScriptMessageType type,
      const base::string16& message_text,
      const base::string16& default_prompt_text,
      const content::JavaScriptDialogManager::DialogClosedCallback& callback);
  ~JavaScriptDialogHost() override;

  bool Show();

  void Dismiss();

  void Handle(bool success, const base::string16* prompt_override);

 private:
  // JavaScriptDialogClient implementation
  void Close(bool success, const base::string16& user_input) override;

  bool is_before_unload_dialog_;
  content::JavaScriptDialogManager::DialogClosedCallback callback_;

  std::unique_ptr<JavaScriptDialog> dialog_;

  DISALLOW_COPY_AND_ASSIGN(JavaScriptDialogHost);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_JAVASCRIPT_DIALOGS_JAVASCRIPT_DIALOG_HOST_H_
