// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_JAVASCRIPT_DIALOGS_JAVASCRIPT_DIALOG_CONTENTS_HELPER_H_
#define _OXIDE_SHARED_BROWSER_JAVASCRIPT_DIALOGS_JAVASCRIPT_DIALOG_CONTENTS_HELPER_H_

#include <memory>

#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string16.h"
#include "content/public/browser/javascript_dialog_manager.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

#include "shared/common/oxide_shared_export.h"

namespace content {
class RenderWidgetHostView;
}

namespace oxide {

class JavaScriptDialogFactory;
class JavaScriptDialogHost;

class OXIDE_SHARED_EXPORT JavaScriptDialogContentsHelper
    : public content::JavaScriptDialogManager,
      public content::WebContentsUserData<JavaScriptDialogContentsHelper>,
      public content::WebContentsObserver {
 public:
  static void CreateForWebContents(content::WebContents* contents);
  static JavaScriptDialogContentsHelper* FromWebContents(
      content::WebContents* contents);

  ~JavaScriptDialogContentsHelper() override;

  JavaScriptDialogFactory* factory() const { return factory_; }

  void set_factory(JavaScriptDialogFactory* factory) {
    factory_ = factory;
  }

  base::WeakPtr<JavaScriptDialogContentsHelper> GetWeakPtr();

 private:
  friend class content::WebContentsUserData<JavaScriptDialogContentsHelper>;

  JavaScriptDialogContentsHelper(content::WebContents* contents);

  void ReportDialogUnsupported(content::JavaScriptMessageType type) const;
  bool IsWebContentsForeground() const;
  void DismissActiveDialog();
  void DismissPendingOrActiveDialog();
  void RunPendingDialog(
      const DialogClosedCallback& callback = DialogClosedCallback());
  void OnDialogClosed(const DialogClosedCallback& callback,
                      bool success,
                      const base::string16& user_input);
  void HandleFocusOrVisibilityChange();

  // content::JavaScriptDialogManager implementation
  void RunJavaScriptDialog(
      content::WebContents* source_web_contents,
      const GURL& origin_url,
      content::JavaScriptMessageType javascript_message_type,
      const base::string16& message_text,
      const base::string16& default_prompt_text,
      const DialogClosedCallback& callback,
      bool* did_suppress_message) override;
  void RunBeforeUnloadDialog(content::WebContents* source_web_contents,
                             bool is_reload,
                             bool is_renderer_initiated,
                             bool has_user_gesture,
                             const DialogClosedCallback& callback,
                             bool* did_suppress_dialog) override;
  bool HandleJavaScriptDialog(content::WebContents* source_web_contents,
                              bool accept,
                              const base::string16* prompt_override) override;
  void CancelDialogs(content::WebContents* source_web_contents,
                     bool reset_state) override;

  // content::WebContentsObserver implementation
  void WasShown() override;
  void WasHidden() override;
  void WebContentsDestroyed() override;
  void OnWebContentsFocused() override;

  JavaScriptDialogFactory* factory_;

  struct DialogRequestData {
    GURL origin_url;
    content::JavaScriptMessageType type = content::JAVASCRIPT_MESSAGE_TYPE_ALERT;
    base::string16 message_text;
    base::string16 default_prompt_text;
  };
  std::unique_ptr<DialogRequestData> pending_dialog_request_data_;

  std::unique_ptr<JavaScriptDialogHost> active_dialog_;

  bool is_displaying_before_unload_dialog_;

  base::WeakPtrFactory<JavaScriptDialogContentsHelper> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(JavaScriptDialogContentsHelper);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_JAVASCRIPT_DIALOGS_JAVASCRIPT_DIALOG_CONTENTS_HELPER_H_
