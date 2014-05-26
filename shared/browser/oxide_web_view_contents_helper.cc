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

#include "oxide_web_view_contents_helper.h"

#include <vector>

#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/page_navigator.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/file_chooser_params.h"
#include "content/public/common/renderer_preferences.h"
#include "ui/base/window_open_disposition.h"
#include "ui/shell_dialogs/selected_file_info.h"

#include "oxide_javascript_dialog_manager.h"
#include "oxide_web_view.h"
#include "oxide_web_view_contents_helper_delegate.h"

namespace oxide {

namespace {
const char kWebViewContentsHelperKey[] = "oxide_web_view_contents_helper_data";
}

#define DCHECK_VALID_SOURCE_CONTENTS DCHECK_EQ(source, web_contents());

WebViewContentsHelper::~WebViewContentsHelper() {
  if (web_contents()) {
    web_contents()->SetDelegate(NULL);
  }
}

void WebViewContentsHelper::NotifyUserAgentStringChanged() {
  // See https://launchpad.net/bugs/1279900 and the comment in
  // HttpUserAgentSettings::GetUserAgent()
  web_contents()->SetUserAgentOverride(context_->GetUserAgent());
}

content::WebContents* WebViewContentsHelper::OpenURLFromTab(
    content::WebContents* source,
    const content::OpenURLParams& params) {
  DCHECK_VALID_SOURCE_CONTENTS

  if (!delegate_) {
    return NULL;
  }

  return delegate_->OpenURL(params);
}

void WebViewContentsHelper::NavigationStateChanged(
    const content::WebContents* source,
    unsigned changed_flags) {
  DCHECK_VALID_SOURCE_CONTENTS

  if (!delegate_) {
    return;
  }

  delegate_->NavigationStateChanged(changed_flags);
}

bool WebViewContentsHelper::ShouldCreateWebContents(
    content::WebContents* source,
    int route_id,
    WindowContainerType window_container_type,
    const base::string16& frame_name,
    const GURL& target_url,
    const std::string& partition_id,
    content::SessionStorageNamespace* session_storage_namespace,
    WindowOpenDisposition disposition,
    bool user_gesture) {
  DCHECK_VALID_SOURCE_CONTENTS

  if (!delegate_) {
    return false;
  }

  return delegate_->ShouldCreateWebContents(target_url,
                                            disposition,
                                            user_gesture);
}

void WebViewContentsHelper::WebContentsCreated(
    content::WebContents* source,
    int source_frame_id,
    const base::string16& frame_name,
    const GURL& target_url,
    content::WebContents* new_contents) {
  DCHECK_VALID_SOURCE_CONTENTS
  DCHECK(!WebView::FromWebContents(new_contents));

  new WebViewContentsHelper(new_contents);
}

void WebViewContentsHelper::AddNewContents(content::WebContents* source,
                                           content::WebContents* new_contents,
                                           WindowOpenDisposition disposition,
                                           const gfx::Rect& initial_pos,
                                           bool user_gesture,
                                           bool* was_blocked) {
  DCHECK_VALID_SOURCE_CONTENTS
  DCHECK(disposition == NEW_FOREGROUND_TAB ||
         disposition == NEW_BACKGROUND_TAB ||
         disposition == NEW_POPUP ||
         disposition == NEW_WINDOW) << "Invalid disposition";
  DCHECK_EQ(context_,
            BrowserContext::FromContent(new_contents->GetBrowserContext()));
  DCHECK(delegate_);

  if (was_blocked) {
    *was_blocked = false;
  }

  if (!delegate_->CreateNewViewAndAdoptWebContents(
          make_scoped_ptr(new_contents),
          user_gesture ? disposition : NEW_POPUP,
          initial_pos) && was_blocked) {
    *was_blocked = true;
  }
}

void WebViewContentsHelper::LoadProgressChanged(
    content::WebContents* source,
    double progress) {
  DCHECK_VALID_SOURCE_CONTENTS

  if (!delegate_) {
    return;
  }

  // XXX: Should probably save the progress here so that we can initialize
  //  an adopting webview with the current value

  delegate_->LoadProgressChanged(progress);
}

bool WebViewContentsHelper::AddMessageToConsole(
    content::WebContents* source,
    int32 level,
    const base::string16& message,
    int32 line_no,
    const base::string16& source_id) {
  DCHECK_VALID_SOURCE_CONTENTS

  if (delegate_) {
    delegate_->AddMessageToConsole(level, message, line_no, source_id);
  }

  return true;
}

content::JavaScriptDialogManager*
WebViewContentsHelper::GetJavaScriptDialogManager() {
  return JavaScriptDialogManager::GetInstance();
}

void WebViewContentsHelper::RunFileChooser(
    content::WebContents* source,
    const content::FileChooserParams& params) {
  DCHECK_VALID_SOURCE_CONTENTS

  if (!delegate_ || !delegate_->RunFileChooser(params)) {
    std::vector<ui::SelectedFileInfo> empty;
    source->GetRenderViewHost()->FilesSelectedInChooser(empty, params.mode);
  }
}

void WebViewContentsHelper::ToggleFullscreenModeForTab(
    content::WebContents* source,
    bool enter) {
  DCHECK_VALID_SOURCE_CONTENTS

  if (!delegate_) {
    return;
  }

  delegate_->ToggleFullscreenMode(enter);
}

bool WebViewContentsHelper::IsFullscreenForTabOrPending(
    const content::WebContents* source) const {
  DCHECK_VALID_SOURCE_CONTENTS

  WebView* view = WebView::FromWebContents(source);
  if (!view) {
    return false;
  }

  return view->IsFullscreen();
}

WebViewContentsHelper::WebViewContentsHelper(content::WebContents* contents)
    : BrowserContextObserver(BrowserContext::FromContent(contents->GetBrowserContext())),
      content::WebContentsObserver(contents),
      context_(BrowserContext::FromContent(contents->GetBrowserContext())),
      delegate_(NULL) {
  DCHECK(!FromWebContents(contents));

  contents->SetDelegate(this);
  contents->SetUserData(kWebViewContentsHelperKey, this);

  // This must come before SetUserAgentOverride, as we rely on that to
  // to sync this to the renderer, if it already exists
  content::RendererPreferences* renderer_prefs =
      web_contents()->GetMutableRendererPrefs();
  renderer_prefs->browser_handles_non_local_top_level_requests = true;

  // See https://launchpad.net/bugs/1279900 and the comment in
  // HttpUserAgentSettings::GetUserAgent()
  web_contents()->SetUserAgentOverride(context_->GetUserAgent());
}

// static
WebViewContentsHelper* WebViewContentsHelper::FromWebContents(
    content::WebContents* contents) {
  return static_cast<WebViewContentsHelper *>(
      contents->GetUserData(kWebViewContentsHelperKey));
}

void WebViewContentsHelper::SetDelegate(
    WebViewContentsHelperDelegate* delegate) {
  delegate_ = delegate;
}

void WebViewContentsHelper::LoadURLWithParams(
    const content::NavigationController::LoadURLParams& params) {
  content::NavigationController::LoadURLParams p(params);
  // See https://launchpad.net/bugs/1279900 and the comment in
  // HttpUserAgentSettings::GetUserAgent()
  p.override_user_agent = content::NavigationController::UA_OVERRIDE_TRUE;
  web_contents()->GetController().LoadURLWithParams(p);
}

} // namespace oxide
