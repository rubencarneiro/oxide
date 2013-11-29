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

#ifndef _OXIDE_SHARED_BROWSER_JAVASCRIPT_DIALOG_MANAGER_H_
#define _OXIDE_SHARED_BROWSER_JAVASCRIPT_DIALOG_MANAGER_H_

#include "content/public/browser/javascript_dialog_manager.h"

namespace oxide {

class JavaScriptDialogManager : public content::JavaScriptDialogManager {
 public:
  void RunJavaScriptDialog(
      content::WebContents* web_contents,
      const GURL& origin_url,
      const std::string& accept_lang,
      content::JavaScriptMessageType javascript_message_type,
      const string16& message_text,
      const string16& default_prompt_text,
      const DialogClosedCallback& callback,
      bool* did_suppress_message) FINAL;

  void RunBeforeUnloadDialog(content::WebContents* web_contents,
                             const string16& message_text,
                             bool is_reload,
                             const DialogClosedCallback& callback) FINAL;

  bool HandleJavaScriptDialog(content::WebContents* web_contents,
                              bool accept,
                              const string16* prompt_override) FINAL;

  void CancelActiveAndPendingDialogs(content::WebContents* web_contents) FINAL;

  void WebContentsDestroyed(content::WebContents* web_contents) FINAL;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_JAVASCRIPT_DIALOG_MANAGER_H_
