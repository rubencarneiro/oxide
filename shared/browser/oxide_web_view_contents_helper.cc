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

#include "oxide_web_view_contents_helper.h"

#include <set>

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/renderer_preferences.h"
#include "ui/display/display.h"

#include "shared/common/oxide_content_client.h"

#include "oxide_browser_context.h"
#include "oxide_web_contents_unloader.h"
#include "oxide_web_contents_view.h"
#include "oxide_web_preferences.h"
#include "oxide_web_view.h"

namespace oxide {

namespace {
const char kWebViewContentsHelperKey[] = "oxide_web_view_contents_helper_data";
base::LazyInstance<std::set<WebViewContentsHelper*>> g_contents_helpers =
    LAZY_INSTANCE_INITIALIZER;
}

WebViewContentsHelper::~WebViewContentsHelper() {
  size_t erased = g_contents_helpers.Get().erase(this);
  DCHECK_GT(erased, 0U);

  if (web_preferences() && owns_web_preferences_) {
    WebPreferences* prefs = web_preferences();
    WebPreferencesObserver::Observe(nullptr);
    prefs->Destroy();
  }
}

void WebViewContentsHelper::UpdateWebPreferences() {
  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->OnWebkitPreferencesChanged();
}

void WebViewContentsHelper::NotifyPopupBlockerEnabledChanged() {
  UpdateWebPreferences();
}

void WebViewContentsHelper::NotifyDoNotTrackChanged() {
  content::RendererPreferences* renderer_prefs =
      web_contents_->GetMutableRendererPrefs();
  renderer_prefs->enable_do_not_track = GetBrowserContext()->GetDoNotTrack();

  // Send the new override string to the renderer.
  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  if (!rvh) {
    return;
  }
  rvh->SyncRendererPrefs();
}

void WebViewContentsHelper::OnDisplayPropertiesChanged(
    const display::Display& display) {
  if (display.id() !=
      WebContentsView::FromWebContents(web_contents_)->GetDisplay().id()) {
    return;
  }

  UpdateWebPreferences();
}

void WebViewContentsHelper::OnShellModeChanged() {
  UpdateWebPreferences();
}

void WebViewContentsHelper::WebPreferencesValueChanged() {
  UpdateWebPreferences();
}

WebViewContentsHelper::WebViewContentsHelper(content::WebContents* contents,
                                             content::WebContents* opener)
    : BrowserContextObserver(
          BrowserContext::FromContent(contents->GetBrowserContext())),
      web_contents_(contents),
      owns_web_preferences_(false) {
  DCHECK(!FromWebContents(web_contents_));

  g_contents_helpers.Get().insert(this);

  web_contents_->SetUserData(kWebViewContentsHelperKey, this);

  content::RendererPreferences* renderer_prefs =
      web_contents_->GetMutableRendererPrefs();
  renderer_prefs->enable_do_not_track = GetBrowserContext()->GetDoNotTrack();

  // Hardcoded selection colors to match the current Ambiance theme from the
  // Ubuntu UI Toolkit (https://bazaar.launchpad.net/~ubuntu-sdk-team/ubuntu-ui-toolkit/trunk/view/head:/src/Ubuntu/Components/Themes/Ambiance/1.3/Palette.qml)
  // TODO: connect to the theme engine
  renderer_prefs->active_selection_bg_color = 0xFFCFECF7;
  renderer_prefs->active_selection_fg_color = 0xFF5D5D5D;
  renderer_prefs->inactive_selection_bg_color = 0xFFCFECF7;
  renderer_prefs->inactive_selection_fg_color = 0xFF5D5D5D;

  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  if (rvh) {
    rvh->SyncRendererPrefs();
  }

  if (opener) {
    WebPreferencesObserver::Observe(
        WebViewContentsHelper::FromWebContents(opener)
          ->GetWebPreferences()->Clone());
    owns_web_preferences_ = true;
    UpdateWebPreferences();
  }
}

// static
WebViewContentsHelper* WebViewContentsHelper::FromWebContents(
    content::WebContents* contents) {
  if (!contents) {
    return nullptr;
  }
  return static_cast<WebViewContentsHelper *>(
      contents->GetUserData(kWebViewContentsHelperKey));
}

WebViewContentsHelper* WebViewContentsHelper::FromRenderViewHost(
    content::RenderViewHost* rvh) {
  content::WebContents* contents =
      content::WebContents::FromRenderViewHost(rvh);
  if (!contents) {
    return nullptr;
  }

  return FromWebContents(contents);
}

// static
bool WebViewContentsHelper::IsContextInUse(BrowserContext* context) {
  for (auto* helper : g_contents_helpers.Get()) {
    if (helper->GetBrowserContext() == context &&
        !WebContentsUnloader::GetInstance()->IsUnloadInProgress(
            helper->GetWebContents())) {
      return true;
    }
  }

  return false;
}

content::WebContents* WebViewContentsHelper::GetWebContents() const {
  return web_contents_;
}

BrowserContext* WebViewContentsHelper::GetBrowserContext() const {
  return BrowserContext::FromContent(web_contents_->GetBrowserContext());
}

WebPreferences* WebViewContentsHelper::GetWebPreferences() const {
  return web_preferences();
}

void WebViewContentsHelper::SetWebPreferences(WebPreferences* preferences) {
  if (preferences == web_preferences()) {
    return;
  }

  if (web_preferences() && owns_web_preferences_) {
    WebPreferences* old = web_preferences();
    WebPreferencesObserver::Observe(nullptr);
    old->Destroy();
  }

  owns_web_preferences_ = false;

  WebPreferencesObserver::Observe(preferences);
  UpdateWebPreferences();
}

void WebViewContentsHelper::WebContentsAdopted() {
  owns_web_preferences_ = false;
}

} // namespace oxide
