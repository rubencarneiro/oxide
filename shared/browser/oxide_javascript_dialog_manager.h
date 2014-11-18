// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2014 Canonical Ltd.

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

#include <deque>
#include <map>

#include "content/public/browser/javascript_dialog_manager.h"

namespace oxide {

class JavaScriptDialog;

class JavaScriptDialogManager : public content::JavaScriptDialogManager {
 public:
  static JavaScriptDialogManager* GetInstance();

  void RunJavaScriptDialog(
      content::WebContents* web_contents,
      const GURL& origin_url,
      const std::string& accept_lang,
      content::JavaScriptMessageType javascript_message_type,
      const base::string16& message_text,
      const base::string16& default_prompt_text,
      const content::JavaScriptDialogManager::DialogClosedCallback& callback,
      bool* did_suppress_message) final;

  void RunBeforeUnloadDialog(content::WebContents* web_contents,
                             const base::string16& message_text,
                             bool is_reload,
                             const DialogClosedCallback& callback) final;

  bool HandleJavaScriptDialog(content::WebContents* web_contents,
                              bool accept,
                              const base::string16* prompt_override) final;

  void CancelActiveAndPendingDialogs(content::WebContents* web_contents) final;

  void WebContentsDestroyed(content::WebContents* web_contents) final;

 private:
  friend class JavaScriptDialog;

  // Note: chrome implements application-modal dialogs (only one dialog active
  // for all windows and tabs at any given time), whereas we do tab-modal
  // dialogs. See https://code.google.com/p/chromium/issues/detail?id=456.
  std::map<content::WebContents*, JavaScriptDialog*> active_dialogs_;
  std::map<content::WebContents*, std::deque<JavaScriptDialog*> > queues_;

  JavaScriptDialog* GetActiveDialog(content::WebContents* web_contents) const;

  void OnDialogClosed(content::WebContents* web_contents,
                      JavaScriptDialog* dialog);
  void OnDialogCancelled(content::WebContents* web_contents,
                         JavaScriptDialog* dialog);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_JAVASCRIPT_DIALOG_MANAGER_H_
