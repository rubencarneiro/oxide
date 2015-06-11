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

#include "base/logging.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/renderer_preferences.h"

#include "shared/common/oxide_content_client.h"

#include "oxide_browser_context.h"
#include "oxide_web_preferences.h"
#include "oxide_web_view.h"

namespace oxide {

namespace {
const char kWebViewContentsHelperKey[] = "oxide_web_view_contents_helper_data";
}

WebViewContentsHelper::~WebViewContentsHelper() {
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

void WebViewContentsHelper::WebPreferencesValueChanged() {
  UpdateWebPreferences();
}

void WebViewContentsHelper::NotifyDoNotTrackChanged() {
  content::RendererPreferences* renderer_prefs =
      web_contents_->GetMutableRendererPrefs();
  renderer_prefs->enable_do_not_track = context_->GetDoNotTrack();

  // Send the new override string to the renderer.
  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  if (!rvh) {
    return;
  }
  rvh->SyncRendererPrefs();
}

WebViewContentsHelper::WebViewContentsHelper(content::WebContents* contents,
                                             content::WebContents* opener)
    : BrowserContextObserver(
          BrowserContext::FromContent(contents->GetBrowserContext())),
      context_(BrowserContext::FromContent(contents->GetBrowserContext())),
      web_contents_(contents),
      owns_web_preferences_(false) {
  DCHECK(!FromWebContents(web_contents_));

  web_contents_->SetUserData(kWebViewContentsHelperKey, this);

  content::RendererPreferences* renderer_prefs =
      web_contents_->GetMutableRendererPrefs();
  renderer_prefs->browser_handles_non_local_top_level_requests = true;
  renderer_prefs->enable_do_not_track = context_->GetDoNotTrack();

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
  return static_cast<WebViewContentsHelper *>(
      contents->GetUserData(kWebViewContentsHelperKey));
}

// static
WebViewContentsHelper* WebViewContentsHelper::FromRenderViewHost(
    content::RenderViewHost* rvh) {
  return FromWebContents(content::WebContents::FromRenderViewHost(rvh));
}

content::WebContents* WebViewContentsHelper::GetWebContents() const {
  return web_contents_;
}

BrowserContext* WebViewContentsHelper::GetBrowserContext() const {
  return context_.get();
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
