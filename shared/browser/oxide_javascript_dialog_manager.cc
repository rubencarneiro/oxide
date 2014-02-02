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

#include "oxide_javascript_dialog_manager.h"

#include "base/memory/singleton.h"

#include "shared/browser/oxide_javascript_dialog.h"
#include "shared/browser/oxide_web_view.h"

namespace oxide {

JavaScriptDialogManager* JavaScriptDialogManager::GetInstance() {
  return Singleton<JavaScriptDialogManager>::get();
}

void JavaScriptDialogManager::RunJavaScriptDialog(
    content::WebContents* web_contents,
    const GURL& origin_url,
    const std::string& accept_lang,
    content::JavaScriptMessageType javascript_message_type,
    const base::string16& message_text,
    const base::string16& default_prompt_text,
    const content::JavaScriptDialogManager::DialogClosedCallback& callback,
    bool* did_suppress_message) {
  WebView* webview = WebView::FromWebContents(web_contents);
  JavaScriptDialog* dialog = webview->CreateJavaScriptDialog(
      javascript_message_type, did_suppress_message);
  if (!dialog) {
    callback.Run(false, base::string16());
    return;
  }
  dialog->web_contents_ = web_contents;
  dialog->origin_url_ = origin_url;
  dialog->accept_lang_ = accept_lang;
  dialog->message_text_ = message_text;
  dialog->default_prompt_text_ = default_prompt_text;
  dialog->callback_ = callback;
  if (GetActiveDialog(web_contents)) {
    std::deque<JavaScriptDialog*>& queue = queues_[web_contents];
    queue.push_back(dialog);
  } else {
    active_dialogs_[web_contents] = dialog;
    dialog->Run();
  }
}

void JavaScriptDialogManager::RunBeforeUnloadDialog(
    content::WebContents* web_contents,
    const base::string16& message_text,
    bool is_reload,
    const DialogClosedCallback& callback) {
  WebView* webview = WebView::FromWebContents(web_contents);
  JavaScriptDialog* dialog = webview->CreateBeforeUnloadDialog();
  if (!dialog) {
    callback.Run(true, base::string16());
    return;
  }
  dialog->web_contents_ = web_contents;
  dialog->message_text_ = message_text;
  dialog->is_reload_ = is_reload;
  dialog->is_before_unload_dialog_ = true;
  dialog->callback_ = callback;
  if (GetActiveDialog(web_contents)) {
    std::deque<JavaScriptDialog*>& queue = queues_[web_contents];
    queue.push_back(dialog);
  } else {
    active_dialogs_[web_contents] = dialog;
    dialog->Run();
  }
}

bool JavaScriptDialogManager::HandleJavaScriptDialog(
    content::WebContents* web_contents,
    bool accept,
    const base::string16* prompt_override) {
  JavaScriptDialog* dialog = GetActiveDialog(web_contents);
  if (!dialog) {
    return false;
  }
  dialog->Handle(accept, prompt_override);
  return true;
}

void JavaScriptDialogManager::CancelActiveAndPendingDialogs(
    content::WebContents* web_contents) {
  HandleJavaScriptDialog(web_contents, false, 0);
  // TODO!
}

void JavaScriptDialogManager::WebContentsDestroyed(
    content::WebContents* web_contents) {
  // TODO!
}

JavaScriptDialog* JavaScriptDialogManager::GetActiveDialog(
    content::WebContents* web_contents) const {
  std::map<content::WebContents*, JavaScriptDialog*>::const_iterator it;
  it = active_dialogs_.find(web_contents);
  if (it == active_dialogs_.end()) {
    return NULL;
  }
  return it->second;
}

void JavaScriptDialogManager::OnDialogClosed(
    content::WebContents* web_contents,
    JavaScriptDialog* dialog) {
  DCHECK_EQ(dialog, GetActiveDialog(web_contents));
  active_dialogs_[web_contents] = NULL;
  delete dialog;
  std::deque<JavaScriptDialog*>& queue = queues_[web_contents];
  if (!queue.empty()) {
    JavaScriptDialog* next_dialog = queue.front();
    queue.pop_front();
    active_dialogs_[web_contents] = next_dialog;
    next_dialog->Run();
  }
}

} // namespace oxide
