// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#include "chrome_controller.h"

#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/page_type.h"
#include "content/public/common/url_constants.h"
#include "url/gurl.h"

#include "shared/browser/compositor/oxide_compositor.h"
#include "shared/browser/ssl/oxide_security_types.h"
#include "shared/common/oxide_messages.h"

#include "chrome_controller_client.h"
#include "oxide_fullscreen_helper.h"
#include "oxide_render_widget_host_view.h"
#include "oxide_web_contents_view.h"

namespace oxide {

DEFINE_WEB_CONTENTS_USER_DATA_KEY(ChromeController);

ChromeController::ChromeController(content::WebContents* contents)
    : content::WebContentsObserver(contents),
      client_(nullptr),
      top_controls_height_(0),
      constraints_(blink::WebBrowserControlsBoth),
      animation_enabled_(true) {
  CompositorObserver::Observe(
      WebContentsView::FromWebContents(contents)->GetCompositor());

  FullscreenHelper::CreateForWebContents(contents);
  SecurityStatus::CreateForWebContents(contents);
  WebProcessStatusMonitor::CreateForWebContents(contents);

  security_status_subscription_ =
      SecurityStatus::FromWebContents(contents)->AddChangeCallback(
          base::Bind(&ChromeController::OnSecurityStatusChanged,
                     base::Unretained(this)));
  web_process_status_subscription_ =
      WebProcessStatusMonitor::FromWebContents(contents)->AddChangeCallback(
          base::Bind(&ChromeController::OnWebProcessStatusChanged,
                     base::Unretained(this)));

  InitializeForHost(contents->GetMainFrame(), true);
}

void ChromeController::InitializeForHost(
    content::RenderFrameHost* render_frame_host,
    bool initial_host) {
  // Show the location bar if this is the initial RVH and the constraints
  // are set to blink::WebBrowserControlsBoth
  blink::WebBrowserControlsState current = constraints_;
  if (initial_host && constraints_ == blink::WebBrowserControlsBoth) {
    current = blink::WebBrowserControlsShown;
  }

  UpdateBrowserControlsState(render_frame_host, current, false);
}

void ChromeController::RefreshBrowserControlsState() {
  UpdateBrowserControlsState(web_contents()->GetMainFrame(),
                             blink::WebBrowserControlsBoth,
                             animation_enabled_);
}

void ChromeController::UpdateBrowserControlsState(
    content::RenderFrameHost* render_frame_host,
    blink::WebBrowserControlsState current_state,
    bool animated) {
  blink::WebBrowserControlsState constraints = constraints_;
  if (constraints_ == blink::WebBrowserControlsBoth) {
    if (!CanHideBrowserControls()) {
      current_state = constraints = blink::WebBrowserControlsShown;
    } else if (!CanShowBrowserControls()) {
      current_state = constraints = blink::WebBrowserControlsHidden;
    }
  }

  DCHECK((current_state != blink::WebBrowserControlsHidden ||
          constraints != blink::WebBrowserControlsShown) &&
         (current_state != blink::WebBrowserControlsShown ||
          constraints != blink::WebBrowserControlsHidden));

  // render_frame_host can be null here, because I think we're hitting something
  // like https://bugs.chromium.org/p/chromium/issues/detail?id=575245
  // (RenderViewHost::GetMainFrame is returning nullptr inside
  // RenderViewHostChanged)
  if (!render_frame_host) {
    render_frame_host = web_contents()->GetMainFrame();
  }
  content::RenderViewHost* rvh = render_frame_host->GetRenderViewHost();

  rvh->Send(
      new OxideMsg_UpdateBrowserControlsState(rvh->GetRoutingID(),
                                          constraints,
                                          current_state,
                                          animated));

  if (render_frame_host->IsRenderFrameLive() && !RendererIsUnresponsive()) {
    return;
  }

  committed_frame_metadata_ = DefaultMetadata();
  WebContentsView::FromWebContents(web_contents())
      ->GetCompositor()
      ->SetNeedsRedraw();
}

RenderWidgetHostView* ChromeController::GetRenderWidgetHostView() {
  content::RenderWidgetHostView* rwhv =
      web_contents()->GetFullscreenRenderWidgetHostView();
  if (!rwhv) {
    rwhv = web_contents()->GetRenderWidgetHostView();
  }

  return static_cast<RenderWidgetHostView*>(rwhv);
}

content::RenderWidgetHost* ChromeController::GetRenderWidgetHost() {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return nullptr;
  }

  return rwhv->GetRenderWidgetHost();
}

bool ChromeController::RendererIsUnresponsive() const {
  WebProcessStatusMonitor* status_monitor =
      WebProcessStatusMonitor::FromWebContents(web_contents());
  return status_monitor->GetStatus() ==
         WebProcessStatusMonitor::Status::Unresponsive;
}

cc::CompositorFrameMetadata ChromeController::DefaultMetadata() const {
  cc::CompositorFrameMetadata metadata;
  metadata.top_controls_height = top_controls_height();
  metadata.top_controls_shown_ratio =
      constraints_ == blink::WebBrowserControlsHidden ? 0.f : 1.f;

  return std::move(metadata);
}

bool ChromeController::CanHideBrowserControls() const {
  SecurityStatus* security_status =
      SecurityStatus::FromWebContents(web_contents());
  if (security_status->security_level() == SECURITY_LEVEL_WARNING ||
      security_status->security_level() == SECURITY_LEVEL_ERROR) {
    return false;
  }

  GURL url = web_contents()->GetLastCommittedURL();
  if (!url.is_empty() && url.SchemeIs(content::kChromeUIScheme)) {
    return false;
  }

  if (web_contents()->ShowingInterstitialPage()) {
    return false;
  }

  content::NavigationEntry* entry =
      web_contents()->GetController().GetLastCommittedEntry();
  if (entry && entry->GetPageType() == content::PAGE_TYPE_ERROR) {
    return false;
  }

  if (WebProcessStatusMonitor::FromWebContents(web_contents())->GetStatus() !=
          WebProcessStatusMonitor::Status::Running) {
    return false;
  }

  // TODO(chrisccoulson):
  //  - Chrome/Android blocks hiding if the focused node is editable
  //  - Block hiding if accessibility is enabled (when we are accessible)

  return true;
}

bool ChromeController::CanShowBrowserControls() const {
  if (FullscreenHelper::FromWebContents(web_contents())->IsFullscreen()) {
    return false;
  }

  return true;
}

void ChromeController::OnSecurityStatusChanged(
    SecurityStatus::ChangedFlags flags) {
  if (!(flags & SecurityStatus::CHANGED_FLAG_SECURITY_LEVEL)) {
    return;
  }

  RefreshBrowserControlsState();
}

void ChromeController::OnWebProcessStatusChanged() {
  RefreshBrowserControlsState();
}

void ChromeController::RenderFrameForInterstitialPageCreated(
    content::RenderFrameHost* render_frame_host) {
  if (render_frame_host->GetParent()) {
    return;
  }

  InitializeForHost(render_frame_host, false);
}

void ChromeController::RenderViewHostChanged(
    content::RenderViewHost* old_host,
    content::RenderViewHost* new_host) {
  InitializeForHost(new_host->GetMainFrame(), !old_host);
}

void ChromeController::DidCommitProvisionalLoadForFrame(
    content::RenderFrameHost* render_frame_host,
    const GURL& url,
    ui::PageTransition transition_type) {
  if (render_frame_host->GetParent()) {
    return;
  }

  RefreshBrowserControlsState();
}

void ChromeController::WebContentsDestroyed() {
  // There's no guarantee we'll be deleted before SecurityStatus or
  // WebProcessStatusMonitor, so clear these now
  security_status_subscription_.reset();
  web_process_status_subscription_.reset();
}

void ChromeController::DidShowFullscreenWidget() {
  RefreshBrowserControlsState();
}

void ChromeController::DidDestroyFullscreenWidget() {
  RefreshBrowserControlsState();
}

void ChromeController::DidToggleFullscreenModeForTab(bool entered_fullscreen,
                                                     bool will_cause_resize) {
  RefreshBrowserControlsState();
}

void ChromeController::DidAttachInterstitialPage() {
  RefreshBrowserControlsState();
}

void ChromeController::DidDetachInterstitialPage() {
  RefreshBrowserControlsState();
}

void ChromeController::CompositorDidCommit() {
  committed_frame_metadata_ =
      GetRenderWidgetHostView() && !RendererIsUnresponsive() ?
          GetRenderWidgetHostView()->last_submitted_frame_metadata().Clone()
          : DefaultMetadata();
}

void ChromeController::CompositorWillRequestSwapFrame() {
  current_frame_metadata_ = committed_frame_metadata_.Clone();

  if (!client_) {
    return;
  }

  client_->ChromePositionUpdated();
}

// static
ChromeController* ChromeController::FromWebContents(
    content::WebContents* contents) {
  return content::WebContentsUserData<ChromeController>::FromWebContents(contents);
}

ChromeController::~ChromeController() = default;

void ChromeController::SetTopControlsHeight(float height) {
  if (height == top_controls_height_) {
    return;
  }

  top_controls_height_ = height;

  content::RenderWidgetHost* host = GetRenderWidgetHost();
  if (!host || RendererIsUnresponsive()) {
    committed_frame_metadata_ = DefaultMetadata();
    WebContentsView::FromWebContents(web_contents())
        ->GetCompositor()
        ->SetNeedsRedraw();
  }

  if (!host) {
    return;
  }

  host->WasResized();
}

void ChromeController::SetConstraints(blink::WebBrowserControlsState constraints) {
  if (constraints == constraints_) {
    return;
  }

  constraints_ = constraints;

  UpdateBrowserControlsState(web_contents()->GetMainFrame(),
                             blink::WebBrowserControlsBoth,
                             animation_enabled_);
}

void ChromeController::Show(bool animate) {
  DCHECK_EQ(constraints_, blink::WebBrowserControlsBoth);

  UpdateBrowserControlsState(web_contents()->GetMainFrame(),
                             blink::WebBrowserControlsShown,
                             animate);
}

void ChromeController::Hide(bool animate) {
  DCHECK_EQ(constraints_, blink::WebBrowserControlsBoth);

  UpdateBrowserControlsState(web_contents()->GetMainFrame(),
                             blink::WebBrowserControlsHidden,
                             animate);
}

float ChromeController::GetTopControlsOffset() const {
  return GetTopContentOffset() - top_controls_height();
}

float ChromeController::GetTopContentOffset() const {
  return current_frame_metadata_.top_controls_height *
         current_frame_metadata_.top_controls_shown_ratio;
}

} // namespace oxide
