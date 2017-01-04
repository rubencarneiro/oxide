// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#include "oxide_navigation_intercept_resource_throttle.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/logging.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/browser/web_contents.h"
#include "url/gurl.h"
#include "net/url_request/url_request.h"
#include "ui/base/page_transition_types.h"

#include "web_contents_client.h"
#include "web_contents_helper.h"

namespace oxide {

namespace {

bool ShouldProceedWithNavigation(const GURL& url,
                                 int render_process_id,
                                 int render_frame_id,
                                 bool has_user_gesture) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  content::RenderFrameHost* render_frame_host =
      content::RenderFrameHost::FromID(render_process_id, render_frame_id);
  if (!render_frame_host) {
    return false;
  }

  content::WebContents* contents =
      content::WebContents::FromRenderFrameHost(render_frame_host);
  if (!contents) {
    return true;
  }

  if (contents->GetController().IsInitialNavigation()) {
    return true;
  }

  WebContentsHelper* helper = WebContentsHelper::FromWebContents(contents);
  if (!helper->client()) {
    return true;
  }

  return helper->client()->ShouldHandleNavigation(url, has_user_gesture);
}

void CheckShouldNavigationProceedOnUIThread(
    const GURL& url,
    int render_process_id,
    int render_frame_id,
    bool has_user_gesture,
    const base::Callback<void(bool)>& callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  bool proceed = ShouldProceedWithNavigation(url,
                                             render_process_id,
                                             render_frame_id,
                                             has_user_gesture);

  content::BrowserThread::PostTask(content::BrowserThread::IO,
                                   FROM_HERE,
                                   base::Bind(callback, proceed));
}

}

void NavigationInterceptResourceThrottle::WillStartRequest(bool* defer) {
  const content::ResourceRequestInfo* info =
      content::ResourceRequestInfo::ForRequest(request_);
  DCHECK(info);

  ui::PageTransition transition = info->GetPageTransition();
  if ((transition & ui::PAGE_TRANSITION_FROM_API) ||
      ui::PageTransitionCoreTypeIs(transition, ui::PAGE_TRANSITION_RELOAD) ||
      (transition & ui::PAGE_TRANSITION_FORWARD_BACK)) {
    // This is a browser-initiated navigation
    return;
  }

  base::Callback<void(bool)> callback =
      base::Bind(
        &NavigationInterceptResourceThrottle::HandleShouldNavigateResponseOnIOThread,
        weak_factory_.GetWeakPtr());

  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&CheckShouldNavigationProceedOnUIThread,
                 request_->url(),
                 info->GetChildID(),
                 info->GetRenderFrameID(),
                 // XXX: Will always be false for renderer-initiated
                 //  CURRENT_TAB navigations that go through
                 //  WebView::OpenURLFromTab. See the comment there
                 info->HasUserGesture(),
                 callback));

  *defer = true;
}

const char* NavigationInterceptResourceThrottle::GetNameForLogging() const {
  return "Oxide_NavigationInterceptThrottle";
}

void NavigationInterceptResourceThrottle::
    HandleShouldNavigateResponseOnIOThread(bool proceed) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

  if (proceed) {
    Resume();
  } else {
    CancelAndIgnore();
  }
}

NavigationInterceptResourceThrottle::NavigationInterceptResourceThrottle(
    net::URLRequest* request)
    : request_(request),
      weak_factory_(this) {}

NavigationInterceptResourceThrottle::~NavigationInterceptResourceThrottle() {}

} // namespace oxide
