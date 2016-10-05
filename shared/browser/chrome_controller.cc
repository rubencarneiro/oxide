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
      constraints_(blink::WebTopControlsBoth),
      animation_enabled_(true),
      renderer_is_unresponsive_(false),
      renderer_crashed_(false) {
  CompositorObserver::Observe(
      WebContentsView::FromWebContents(contents)->GetCompositor());

  FullscreenHelper::CreateForWebContents(contents);
  SecurityStatus::CreateForWebContents(contents);

  security_status_subscription_ =
      SecurityStatus::FromWebContents(contents)->AddChangeCallback(
          base::Bind(&ChromeController::OnSecurityStatusChanged,
                     base::Unretained(this)));

  InitializeForHost(contents->GetMainFrame(), true);
}

void ChromeController::InitializeForHost(
    content::RenderFrameHost* render_frame_host,
    bool initial_host) {
  // Show the location bar if this is the initial RVH and the constraints
  // are set to blink::WebTopControlsBoth
  blink::WebTopControlsState current = constraints_;
  if (initial_host && constraints_ == blink::WebTopControlsBoth) {
    current = blink::WebTopControlsShown;
  }

  UpdateTopControlsState(render_frame_host, current, false);
}

void ChromeController::RefreshTopControlsState() {
  UpdateTopControlsState(web_contents()->GetMainFrame(),
                         blink::WebTopControlsBoth,
                         animation_enabled_);
}

void ChromeController::UpdateTopControlsState(
    content::RenderFrameHost* render_frame_host,
    blink::WebTopControlsState current_state,
    bool animated) {
  blink::WebTopControlsState constraints = constraints_;
  if (constraints_ == blink::WebTopControlsBoth) {
    if (!CanHideTopControls()) {
      current_state = constraints = blink::WebTopControlsShown;
    } else if (!CanShowTopControls()) {
      current_state = constraints = blink::WebTopControlsHidden;
    }
  }

  DCHECK((current_state != blink::WebTopControlsHidden ||
          constraints != blink::WebTopControlsShown) &&
         (current_state != blink::WebTopControlsShown ||
          constraints != blink::WebTopControlsHidden));

  // render_frame_host can be null here, because I think we're hitting something
  // like https://bugs.chromium.org/p/chromium/issues/detail?id=575245
  // (RenderViewHost::GetMainFrame is returning nullptr inside
  // RenderViewHostChanged)
  if (!render_frame_host) {
    render_frame_host = web_contents()->GetMainFrame();
  }
  content::RenderViewHost* rvh = render_frame_host->GetRenderViewHost();

  rvh->Send(
      new OxideMsg_UpdateTopControlsState(rvh->GetRoutingID(),
                                          constraints,
                                          current_state,
                                          animated));

  if (render_frame_host->IsRenderFrameLive() && !renderer_is_unresponsive_) {
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

cc::CompositorFrameMetadata ChromeController::DefaultMetadata() const {
  cc::CompositorFrameMetadata metadata;
  metadata.top_controls_height = top_controls_height();
  metadata.top_controls_shown_ratio =
      constraints_ == blink::WebTopControlsHidden ? 0.f : 1.f;

  return std::move(metadata);
}

bool ChromeController::CanHideTopControls() const {
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

  if (renderer_is_unresponsive_) {
    return false;
  }

  if (web_contents()->GetCrashedStatus() !=
      base::TERMINATION_STATUS_STILL_RUNNING) {
    return false;
  }

  // TODO(chrisccoulson):
  //  - Chrome/Android blocks hiding if the focused node is editable
  //  - Block hiding if accessibility is enabled (when we are accessible)

  return true;
}

bool ChromeController::CanShowTopControls() const {
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

  RefreshTopControlsState();
}

void ChromeController::RenderFrameForInterstitialPageCreated(
    content::RenderFrameHost* render_frame_host) {
  if (render_frame_host->GetParent()) {
    return;
  }

  InitializeForHost(render_frame_host, false);
}

void ChromeController::RenderViewReady() {
  if (!renderer_crashed_) {
    return;
  }

  renderer_crashed_ = false;

  InitializeForHost(web_contents()->GetMainFrame(), false);
}

void ChromeController::RenderProcessGone(base::TerminationStatus status) {
  renderer_crashed_ = true;
  RefreshTopControlsState();
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

  RefreshTopControlsState();
}

void ChromeController::WebContentsDestroyed() {
  // There's no guarantee we'll be deleted before SecurityStatus, so clear this
  // now
  security_status_subscription_.reset();
}

void ChromeController::DidShowFullscreenWidget() {
  RefreshTopControlsState();
}

void ChromeController::DidDestroyFullscreenWidget() {
  RefreshTopControlsState();
}

void ChromeController::DidToggleFullscreenModeForTab(bool entered_fullscreen,
                                                     bool will_cause_resize) {
  RefreshTopControlsState();
}

void ChromeController::DidAttachInterstitialPage() {
  RefreshTopControlsState();
}

void ChromeController::DidDetachInterstitialPage() {
  RefreshTopControlsState();
}

void ChromeController::CompositorDidCommit() {
  committed_frame_metadata_ =
      GetRenderWidgetHostView() && !renderer_is_unresponsive_ ?
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
  if (!host || renderer_is_unresponsive_) {
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

void ChromeController::SetConstraints(blink::WebTopControlsState constraints) {
  if (constraints == constraints_) {
    return;
  }

  constraints_ = constraints;

  UpdateTopControlsState(web_contents()->GetMainFrame(),
                         blink::WebTopControlsBoth,
                         animation_enabled_);
}

void ChromeController::Show(bool animate) {
  DCHECK_EQ(constraints_, blink::WebTopControlsBoth);

  UpdateTopControlsState(web_contents()->GetMainFrame(),
                         blink::WebTopControlsShown,
                         animate);
}

void ChromeController::Hide(bool animate) {
  DCHECK_EQ(constraints_, blink::WebTopControlsBoth);

  UpdateTopControlsState(web_contents()->GetMainFrame(),
                         blink::WebTopControlsHidden,
                         animate);
}

float ChromeController::GetTopControlsOffset() const {
  return GetTopContentOffset() - top_controls_height();
}

float ChromeController::GetTopContentOffset() const {
  return current_frame_metadata_.top_controls_height *
         current_frame_metadata_.top_controls_shown_ratio;
}

void ChromeController::RendererIsResponsive() {
  if (!renderer_is_unresponsive_) {
    return;
  }

  renderer_is_unresponsive_ = false;

  RefreshTopControlsState();
}

void ChromeController::RendererIsUnresponsive() {
  if (renderer_is_unresponsive_) {
    return;
  }

  renderer_is_unresponsive_ = true;

  RefreshTopControlsState();
}

} // namespace oxide
