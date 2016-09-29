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

#include "web_contents_helper.h"

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
#include "oxide_web_view.h"
#include "web_preferences.h"

namespace oxide {

namespace {

DEFINE_WEB_CONTENTS_USER_DATA_KEY(WebContentsHelper);

base::LazyInstance<std::set<WebContentsHelper*>> g_contents_helpers =
    LAZY_INSTANCE_INITIALIZER;

}

void WebContentsDeleter::operator()(content::WebContents* contents) {
  WebContentsUnloader::GetInstance()->Unload(base::WrapUnique(contents));
}

WebContentsHelper::WebContentsHelper(content::WebContents* contents)
    : BrowserContextObserver(
          BrowserContext::FromContent(contents->GetBrowserContext())),
      web_contents_(contents) {
  DCHECK(!FromWebContents(web_contents_));

  g_contents_helpers.Get().insert(this);

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

  SyncRendererPreferences();
}

WebContentsHelper::~WebContentsHelper() {
  size_t erased = g_contents_helpers.Get().erase(this);
  DCHECK_GT(erased, 0U);
}

void WebContentsHelper::SyncRendererPreferences() {
  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->SyncRendererPrefs();
}

void WebContentsHelper::NotifyPopupBlockerEnabledChanged() {
  SyncWebPreferences();
}

void WebContentsHelper::NotifyDoNotTrackChanged() {
  content::RendererPreferences* renderer_prefs =
      web_contents_->GetMutableRendererPrefs();
  renderer_prefs->enable_do_not_track = GetBrowserContext()->GetDoNotTrack();

  SyncRendererPreferences();
}

void WebContentsHelper::OnDisplayPropertiesChanged(
    const display::Display& display) {
  if (display.id() !=
      WebContentsView::FromWebContents(web_contents_)->GetDisplay().id()) {
    return;
  }

  SyncWebPreferences();
}

void WebContentsHelper::OnShellModeChanged() {
  SyncWebPreferences();
}

// static
WebContentsUniquePtr WebContentsHelper::CreateWebContents(
    const content::WebContents::CreateParams& params) {
  WebContentsUniquePtr contents(content::WebContents::Create(params));
  CHECK(contents.get()) << "Failed to create WebContents";

  CreateForWebContents(contents.get(), nullptr);

  return std::move(contents);
}

// static
void WebContentsHelper::CreateForWebContents(content::WebContents* contents,
                                             content::WebContents* opener) {
  content::WebContentsUserData<WebContentsHelper>::CreateForWebContents(contents);
  FromWebContents(contents)->SetPreferences(new WebPreferences());

  if (!opener) {
    return;
  }

  FromWebContents(contents)->SetPreferences(
      FromWebContents(opener)->GetPreferences()->Clone().get());
}

// static
WebContentsHelper* WebContentsHelper::FromRenderViewHost(
    content::RenderViewHost* rvh) {
  content::WebContents* contents =
      content::WebContents::FromRenderViewHost(rvh);
  if (!contents) {
    return nullptr;
  }

  return FromWebContents(contents);
}

// static
bool WebContentsHelper::IsContextInUse(BrowserContext* context) {
  for (auto* helper : g_contents_helpers.Get()) {
    if (helper->GetBrowserContext() == context &&
        !WebContentsUnloader::GetInstance()->IsUnloadInProgress(
            helper->GetWebContents())) {
      return true;
    }
  }

  return false;
}

content::WebContents* WebContentsHelper::GetWebContents() const {
  return web_contents_;
}

BrowserContext* WebContentsHelper::GetBrowserContext() const {
  return BrowserContext::FromContent(web_contents_->GetBrowserContext());
}

WebPreferences* WebContentsHelper::GetPreferences() const {
  DCHECK(preferences_);
  return preferences_.get();
}

void WebContentsHelper::SetPreferences(WebPreferences* preferences) {
  if (preferences == preferences_) {
    return;
  }

  prefs_change_subscription_.reset();

  if (preferences) {
    preferences_ = preferences;
  } else {
    preferences_ = new WebPreferences();
  }

  prefs_change_subscription_ =
      preferences_->AddChangeCallback(
          base::Bind(&WebContentsHelper::SyncWebPreferences,
                     base::Unretained(this)));

  SyncWebPreferences();
}

void WebContentsHelper::SyncWebPreferences() {
  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->OnWebkitPreferencesChanged();
}

} // namespace oxide
