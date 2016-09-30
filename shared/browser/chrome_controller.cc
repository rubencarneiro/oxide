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

#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"

#include "shared/common/oxide_messages.h"

#include "chrome_controller_client.h"
#include "oxide_render_widget_host_view.h"
#include "oxide_web_contents_view.h"

namespace oxide {

DEFINE_WEB_CONTENTS_USER_DATA_KEY(ChromeController);

ChromeController::ChromeController(content::WebContents* contents)
    : client_(nullptr),
      web_contents_(contents),
      top_controls_height_(0),
      constraints_(blink::WebTopControlsBoth),
      animation_enabled_(true) {
  CompositorObserver::Observe(
      WebContentsView::FromWebContents(contents)->GetCompositor());

  content::RenderFrameHost* host = web_contents_->GetMainFrame();
  if (host) {
    InitializeForHost(host, true);
  }
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

  content::RenderViewHost* rvh = render_frame_host->GetRenderViewHost();

  rvh->Send(
      new OxideMsg_UpdateTopControlsState(rvh->GetRoutingID(),
                                          constraints_,
                                          current,
                                          false));
}

RenderWidgetHostView* ChromeController::GetRenderWidgetHostView() {
  content::RenderWidgetHostView* rwhv =
      web_contents_->GetFullscreenRenderWidgetHostView();
  if (!rwhv) {
    rwhv = web_contents_->GetRenderWidgetHostView();
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

content::RenderViewHost* ChromeController::GetRenderViewHost() {
  content::RenderWidgetHost* rwh = GetRenderWidgetHost();
  if (!rwh) {
    return nullptr;
  }

  return content::RenderViewHost::From(rwh);
}

void ChromeController::RenderFrameHostChanged(
    content::RenderFrameHost* old_host,
    content::RenderFrameHost* new_host) {
  if (new_host->GetParent()) {
    return;
  }

  InitializeForHost(new_host, !old_host);
}

void ChromeController::RenderFrameForInterstitialPageCreated(
    content::RenderFrameHost* render_frame_host) {
  if (render_frame_host->GetParent()) {
    return;
  }

  InitializeForHost(render_frame_host, false);
}

void ChromeController::CompositorDidCommit() {
  committed_frame_metadata_ =
      GetRenderWidgetHostView() ?
          GetRenderWidgetHostView()->last_submitted_frame_metadata().Clone()
          : cc::CompositorFrameMetadata();
}

void ChromeController::CompositorWillRequestSwapFrame() {
  current_frame_metadata_ = std::move(committed_frame_metadata_);

  if (!client_) {
    return;
  }

  client_->ChromePositionUpdated();
}

ChromeController::~ChromeController() = default;

void ChromeController::SetTopControlsHeight(float height) {
  if (height == top_controls_height_) {
    return;
  }

  top_controls_height_ = height;

  content::RenderWidgetHost* host = GetRenderWidgetHost();
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

  content::RenderViewHost* rvh = GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->Send(new OxideMsg_UpdateTopControlsState(rvh->GetRoutingID(),
                                                constraints_,
                                                blink::WebTopControlsBoth,
                                                animation_enabled_));
}

void ChromeController::Show(bool animate) {
  DCHECK_EQ(constraints_, blink::WebTopControlsBoth);

  content::RenderViewHost* rvh = GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->Send(new OxideMsg_UpdateTopControlsState(rvh->GetRoutingID(),
                                                constraints_,
                                                blink::WebTopControlsShown,
                                                animate));
}

void ChromeController::Hide(bool animate) {
  DCHECK_EQ(constraints_, blink::WebTopControlsBoth);

  content::RenderViewHost* rvh = GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->Send(new OxideMsg_UpdateTopControlsState(rvh->GetRoutingID(),
                                                constraints_,
                                                blink::WebTopControlsHidden,
                                                animate));
}

float ChromeController::GetTopControlsOffset() const {
  return GetTopContentOffset() - top_controls_height();
}

float ChromeController::GetTopContentOffset() const {
  return current_frame_metadata_.top_controls_height *
         current_frame_metadata_.top_controls_shown_ratio;
}

} // namespace oxide
