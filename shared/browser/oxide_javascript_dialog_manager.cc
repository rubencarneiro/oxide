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

#include "oxide_javascript_dialog_manager.h"

#include "base/memory/singleton.h"

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
    const string16& message_text,
    const string16& default_prompt_text,
    const DialogClosedCallback& callback,
    bool* did_suppress_message) {
  WebView* webview = WebView::FromWebContents(web_contents);
  webview->RunJavaScriptDialog(origin_url, accept_lang, javascript_message_type,
                               message_text, default_prompt_text, callback,
                               did_suppress_message);
}

void JavaScriptDialogManager::RunBeforeUnloadDialog(
    content::WebContents* web_contents,
    const string16& message_text,
    bool is_reload,
    const DialogClosedCallback& callback) {
  WebView* webview = WebView::FromWebContents(web_contents);
  webview->RunBeforeUnloadDialog(message_text, is_reload, callback);
}

bool JavaScriptDialogManager::HandleJavaScriptDialog(
    content::WebContents* web_contents,
    bool accept,
    const string16* prompt_override) {
  // TODO
  return content::JavaScriptDialogManager::HandleJavaScriptDialog(
      web_contents, accept, prompt_override);
}

void JavaScriptDialogManager::CancelActiveAndPendingDialogs(
    content::WebContents* web_contents) {
  // TODO
}

void JavaScriptDialogManager::WebContentsDestroyed(
    content::WebContents* web_contents) {
  // TODO
}

} // namespace oxide
