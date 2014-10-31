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
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/renderer_preferences.h"

#include "shared/common/oxide_content_client.h"

#include "oxide_browser_context.h"
#include "oxide_browser_process_main.h"
#include "oxide_content_browser_client.h"
#include "oxide_web_preferences.h"
#include "oxide_web_view.h"
#include "oxide_web_view_contents_helper_delegate.h"

namespace oxide {

namespace {
const char kWebViewContentsHelperKey[] = "oxide_web_view_contents_helper_data";
}

WebViewContentsHelper::WebViewContentsHelper(content::WebContents* contents)
    : BrowserContextObserver(
          BrowserContext::FromContent(contents->GetBrowserContext())),
      WebPreferencesObserver(
          ContentClient::instance()->browser()->CreateWebPreferences()),
      context_(BrowserContext::FromContent(contents->GetBrowserContext())),
      web_contents_(contents),
      delegate_(NULL),
      owns_web_preferences_(true) {
  CHECK(!FromWebContents(web_contents_));

  web_contents_->SetUserData(kWebViewContentsHelperKey, this);

  content::RendererPreferences* renderer_prefs =
      web_contents_->GetMutableRendererPrefs();
  renderer_prefs->browser_handles_non_local_top_level_requests = true;

  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  if (rvh) {
    rvh->SyncRendererPrefs();
  }
}

WebViewContentsHelper::~WebViewContentsHelper() {
  if (owns_web_preferences_) {
    // Disconnect the observer to prevent it from calling back in to us
    // via a vfunc, which it shouldn't do now we're in our destructor
    WebPreferencesObserver::Observe(NULL);
    delete web_preferences();
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

void WebViewContentsHelper::WebPreferencesDestroyed() {
  CHECK(!owns_web_preferences_) <<
      "Somebody destroyed a WebPreferences owned by us!";
  WebPreferencesObserver::Observe(
      ContentClient::instance()->browser()->CreateWebPreferences());
  owns_web_preferences_ = true;

  WebPreferencesValueChanged();

  if (delegate_) {
    delegate_->NotifyWebPreferencesDestroyed();
  }
}

void WebViewContentsHelper::WebPreferencesValueChanged() {
  UpdateWebPreferences();
}

void WebViewContentsHelper::WebPreferencesAdopted() {
  owns_web_preferences_ = false;
}

void WebViewContentsHelper::CloseContents(content::WebContents* source) {
  DCHECK_EQ(source, web_contents_);
  DCHECK(web_contents_holder_during_close_);

  web_contents_holder_during_close_.reset();
  // |this| has been deleted

  BrowserProcessMain::GetInstance()->DecrementPendingUnloadsCount();
}

// static
void WebViewContentsHelper::Attach(content::WebContents* contents,
                                   content::WebContents* opener) {
  WebViewContentsHelper* helper = new WebViewContentsHelper(contents);
  if (opener) {
    WebViewContentsHelper* opener_helper =
        WebViewContentsHelper::FromWebContents(opener);
    DCHECK(opener_helper);
    helper->GetWebPreferences()->CopyFrom(opener_helper->GetWebPreferences());
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

void WebViewContentsHelper::SetDelegate(
    WebViewContentsHelperDelegate* delegate) {
  delegate_ = delegate;
}

BrowserContext* WebViewContentsHelper::GetBrowserContext() const {
  return context_.get();
}

WebPreferences* WebViewContentsHelper::GetWebPreferences() const {
  return web_preferences();
}

void WebViewContentsHelper::SetWebPreferences(WebPreferences* preferences) {
  CHECK(!preferences || preferences->IsOwnedByEmbedder());
  if (preferences == web_preferences()) {
    return;
  }

  if (web_preferences() && owns_web_preferences_) {
    WebPreferences* old = web_preferences();
    WebPreferencesObserver::Observe(NULL);
    delete old;
  }

  if (preferences) {
    owns_web_preferences_ = false;
  } else {
    preferences = ContentClient::instance()->browser()->CreateWebPreferences();
    owns_web_preferences_ = true;
  }

  WebPreferencesObserver::Observe(preferences);
  WebPreferencesValueChanged();
}

void WebViewContentsHelper::TakeWebContentsOwnershipAndClosePage(
    scoped_ptr<content::WebContents> web_contents) {
  DCHECK_EQ(web_contents.get(), web_contents_);
  DCHECK(!web_contents_holder_during_close_);

  web_contents_holder_during_close_ = web_contents.Pass();

  content::RenderViewHostImpl* rvh =
      static_cast<content::RenderViewHostImpl*>(
        web_contents_->GetRenderViewHost());
  if (!rvh || rvh->SuddenTerminationAllowed()) {
    web_contents_holder_during_close_.reset();
    // |this| has been deleted
    return;
  }

  BrowserProcessMain::GetInstance()->IncrementPendingUnloadsCount();

  web_contents_->SetDelegate(this);
  rvh->ClosePage();
}

} // namespace oxide
