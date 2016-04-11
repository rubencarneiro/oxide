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

#include "oxide_fullscreen_helper.h"

#include "base/logging.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"

#include "oxide_fullscreen_helper_client.h"

namespace oxide {

DEFINE_WEB_CONTENTS_USER_DATA_KEY(FullscreenHelper);

FullscreenHelper::FullscreenHelper(content::WebContents* contents)
    : web_contents_(contents),
      client_(nullptr),
      fullscreen_requested_(false),
      fullscreen_granted_(false) {}

FullscreenHelper::~FullscreenHelper() {}

// static
FullscreenHelper* FullscreenHelper::FromWebContents(
    content::WebContents* contents) {
  return content::WebContentsUserData<FullscreenHelper>
      ::FromWebContents(contents);
}

void FullscreenHelper::SetFullscreenGranted(bool granted) {
  DCHECK(client_);

  if (granted == fullscreen_granted_) {
    return;
  }

  bool was_fullscreen = IsFullscreen();
  // It's important to do this before calling WebContents::ExitFullscreen,
  // as this calls back in to ExitFullscreenMode. If the application
  // calls us synchronously, then we'll run out of stack
  fullscreen_granted_ = granted;
  bool is_fullscreen = IsFullscreen();

  if (is_fullscreen == was_fullscreen) {
    return;
  }

  if (is_fullscreen) {
    content::RenderWidgetHostView* rwhv =
        web_contents_->GetFullscreenRenderWidgetHostView();
    if (!rwhv) {
      rwhv = web_contents_->GetRenderWidgetHostView();
    }
    if (rwhv) {
      content::RenderWidgetHost* host = rwhv->GetRenderWidgetHost();
      if (host) {
        host->WasResized();
      }
    }
  } else {
    web_contents_->ExitFullscreen(false);
  }
}

bool FullscreenHelper::IsFullscreen() const {
  return fullscreen_requested_ && fullscreen_granted_;
}

void FullscreenHelper::EnterFullscreenMode(const GURL& origin) {
  fullscreen_requested_ = true;

  if (fullscreen_granted_) {
    // Nothing to do here. Note, RenderFrameHostImpl::OnToggleFullscreen will
    // send the resize message
    return;
  }

  if (!client_) {
    return;
  }

  client_->EnterFullscreenMode(origin);
}

void FullscreenHelper::ExitFullscreenMode() {
  fullscreen_requested_ = false;

  if (!fullscreen_granted_) {
    return;
  }

  if (!client_) {
    return;
  }

  client_->ExitFullscreenMode();
}

} // namespace oxide
