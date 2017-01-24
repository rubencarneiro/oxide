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

#include "base/macros.h"
#include "content/public/browser/javascript_dialog_manager.h"

namespace base {
template <typename T> struct DefaultSingletonTraits;
}

namespace oxide {

class JavaScriptDialog;

class JavaScriptDialogManager : public content::JavaScriptDialogManager {
 public:
  static JavaScriptDialogManager* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<JavaScriptDialogManager>;
  friend class JavaScriptDialog;

  JavaScriptDialogManager();

  void DialogClosed(content::WebContents* web_contents,
                    JavaScriptDialog* dialog);
  void RunNextDialogForContents(content::WebContents* contents);

  // content::JavaScriptDialogManager implementation
  void RunJavaScriptDialog(
      content::WebContents* web_contents,
      const GURL& origin_url,
      content::JavaScriptMessageType javascript_message_type,
      const base::string16& message_text,
      const base::string16& default_prompt_text,
      const DialogClosedCallback& callback,
      bool* did_suppress_message) override;
  void RunBeforeUnloadDialog(content::WebContents* web_contents,
                             bool is_reload,
                             const DialogClosedCallback& callback) override;
  bool HandleJavaScriptDialog(content::WebContents* web_contents,
                              bool accept,
                              const base::string16* prompt_override) override;
  void CancelDialogs(content::WebContents* web_contents,
                     bool reset_state) override;

  // Note: chrome implements application-modal dialogs (only one dialog active
  // for all windows and tabs at any given time), whereas we do tab-modal
  // dialogs. See https://code.google.com/p/chromium/issues/detail?id=456.
  struct WebContentsData {
    WebContentsData() : active(nullptr) {}

    JavaScriptDialog* active;
    std::deque<JavaScriptDialog*> queue;
  };

  std::map<content::WebContents*, WebContentsData> web_contents_data_;

  DISALLOW_COPY_AND_ASSIGN(JavaScriptDialogManager);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_JAVASCRIPT_DIALOG_MANAGER_H_
