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

#ifndef _OXIDE_SHARED_BROWSER_JAVASCRIPT_DIALOG_H_
#define _OXIDE_SHARED_BROWSER_JAVASCRIPT_DIALOG_H_

#include "content/public/browser/javascript_dialog_manager.h"

namespace oxide {

class JavaScriptDialog {
 public:
  virtual ~JavaScriptDialog();

  virtual void Run() = 0;
  void Close(bool accept, const base::string16& user_input = base::string16());
  void CouldNotShow();
  virtual void Handle(bool accept, const base::string16* prompt_override) = 0;
  void Cancel();

 protected:
  friend class JavaScriptDialogManager;

  JavaScriptDialog();

  GURL origin_url_;
  std::string accept_lang_;
  base::string16 message_text_;
  base::string16 default_prompt_text_;
  bool is_reload_;
  bool is_before_unload_dialog_;

private:
  content::WebContents* web_contents_;
  content::JavaScriptDialogManager::DialogClosedCallback callback_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_JAVASCRIPT_DIALOG_H_
