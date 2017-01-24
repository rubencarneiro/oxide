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

#include <algorithm>

#include "base/memory/singleton.h"
#include "content/public/browser/browser_thread.h"

#include "shared/browser/oxide_javascript_dialog.h"
#include "shared/browser/oxide_web_view.h"

namespace oxide {

JavaScriptDialogManager::JavaScriptDialogManager() {}

void JavaScriptDialogManager::DialogClosed(
    content::WebContents* web_contents,
    JavaScriptDialog* dialog) {
  DCHECK(web_contents_data_.find(web_contents) != web_contents_data_.end());
  DCHECK_EQ(dialog, web_contents_data_[web_contents].active);

  web_contents_data_[web_contents].active = nullptr;
  content::BrowserThread::DeleteSoon(
      content::BrowserThread::UI, FROM_HERE, dialog);

  RunNextDialogForContents(web_contents);
}

void JavaScriptDialogManager::RunNextDialogForContents(
    content::WebContents* contents) {
  DCHECK(web_contents_data_.find(contents) != web_contents_data_.end());

  WebContentsData& data = web_contents_data_[contents];

  if (data.active) {
    return;
  }

  while (!data.queue.empty()) {
    JavaScriptDialog* dialog = data.queue.front();
    data.queue.pop_front();

    data.active = dialog;
    if (dialog->Run()) {
      break;
    }
    data.active = nullptr;

    dialog->callback_.Run(dialog->is_before_unload_dialog_, base::string16());
    delete dialog;
  }

  if (!data.active) {
    web_contents_data_.erase(contents);
  }
}

void JavaScriptDialogManager::RunJavaScriptDialog(
    content::WebContents* web_contents,
    const GURL& origin_url,
    content::JavaScriptMessageType javascript_message_type,
    const base::string16& message_text,
    const base::string16& default_prompt_text,
    const DialogClosedCallback& callback,
    bool* did_suppress_message) {
  *did_suppress_message = false;

  WebView* webview = WebView::FromWebContents(web_contents);
  if (!webview) {
    *did_suppress_message = true;
    return;
  }

  JavaScriptDialog* dialog =
      webview->CreateJavaScriptDialog(javascript_message_type);
  if (!dialog) {
    *did_suppress_message = true;
    return;
  }

  dialog->web_contents_ = web_contents;
  dialog->origin_url_ = origin_url;
  dialog->message_text_ = message_text;
  dialog->default_prompt_text_ = default_prompt_text;
  dialog->callback_ = callback;

  web_contents_data_[web_contents].queue.push_back(dialog);

  RunNextDialogForContents(web_contents);
}

void JavaScriptDialogManager::RunBeforeUnloadDialog(
    content::WebContents* web_contents,
    bool is_reload,
    const DialogClosedCallback& callback) {
  WebView* webview = WebView::FromWebContents(web_contents);
  if (!webview) {
    callback.Run(true, base::string16());
    return;
  }

  JavaScriptDialog* dialog = webview->CreateBeforeUnloadDialog();
  if (!dialog) {
    callback.Run(true, base::string16());
    return;
  }

  dialog->web_contents_ = web_contents;
  dialog->is_reload_ = is_reload;
  dialog->is_before_unload_dialog_ = true;
  dialog->callback_ = callback;

  web_contents_data_[web_contents].queue.push_back(dialog);

  RunNextDialogForContents(web_contents);
}

bool JavaScriptDialogManager::HandleJavaScriptDialog(
    content::WebContents* web_contents,
    bool accept,
    const base::string16* prompt_override) {
  if (web_contents_data_.find(web_contents) == web_contents_data_.end()) {
    return false;
  }

  JavaScriptDialog* dialog = web_contents_data_[web_contents].active;
  if (!dialog) {
    return false;
  }

  dialog->Handle(accept, prompt_override);
  return true;
}

void JavaScriptDialogManager::CancelDialogs(content::WebContents* web_contents,
                                            bool reset_state) {
  if (web_contents_data_.find(web_contents) == web_contents_data_.end()) {
    return;
  }

  WebContentsData& data = web_contents_data_[web_contents];

  if (data.active) {
    data.active->Cancel();
    content::BrowserThread::DeleteSoon(
        content::BrowserThread::UI, FROM_HERE, data.active);
    data.active = nullptr;
  }

  while (!data.queue.empty()) {
    JavaScriptDialog* dialog = data.queue.front();
    data.queue.pop_front();
    dialog->callback_.Run(false, base::string16());
    delete dialog;
  }

  web_contents_data_.erase(web_contents);
}

JavaScriptDialogManager* JavaScriptDialogManager::GetInstance() {
  return base::Singleton<JavaScriptDialogManager>::get();
}

} // namespace oxide
