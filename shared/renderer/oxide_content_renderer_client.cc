// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#include "oxide_content_renderer_client.h"

#include <string>

#include "base/command_line.h"
#include "base/strings/stringprintf.h"
#include "cc/trees/layer_tree_settings.h"
#include "content/public/common/url_constants.h"
#include "content/public/common/user_agent.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "net/base/net_module.h"
#include "third_party/WebKit/public/web/WebRuntimeFeatures.h"
#include "third_party/WebKit/public/web/WebSettings.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "ui/native_theme/native_theme_switches.h"

#include "shared/common/chrome_version.h"
#include "shared/common/oxide_constants.h"
#include "shared/common/oxide_net_resource_provider.h"

#include "oxide_renderer_user_agent_settings.h"
#include "oxide_script_message_dispatcher_renderer.h"
#include "oxide_top_controls_handler.h"
#include "oxide_user_script_scheduler.h"
#include "oxide_user_script_slave.h"
#include "oxide_web_content_settings_client.h"

#if defined(ENABLE_MEDIAHUB)
#include "media/oxide_renderer_media_player_manager.h"
#include "media/oxide_web_media_player.h"
#endif

namespace oxide {

void ContentRendererClient::RenderThreadStarted() {
  content::RenderThread* thread = content::RenderThread::Get();

  user_agent_settings_.reset(new RendererUserAgentSettings());
  thread->AddObserver(user_agent_settings_.get());
  
  new UserScriptSlave();

  net::NetModule::SetResourceProvider(NetResourceProvider);
  // Usually enabled only on Android. We want this on mobile, but
  // should be ok everywhere
  blink::WebRuntimeFeatures::enableOrientationEvent(true);
}

void ContentRendererClient::RenderFrameCreated(
    content::RenderFrame* render_frame) {
  new ScriptMessageDispatcherRenderer(render_frame);
  new WebContentSettingsClient(render_frame);
#if defined(ENABLE_MEDIAHUB)
  new RendererMediaPlayerManager(render_frame);
#endif
}

void ContentRendererClient::RenderViewCreated(
    content::RenderView* render_view) {
  new TopControlsHandler(render_view);
  // XXX: This is currently here because RenderFrame proxies the
  //      notifications we're interested in to RenderView. Make this
  //      a RenderFrameObserver when it grows the features we need
  new UserScriptScheduler(render_view);

  blink::WebSettings* settings = render_view->GetWebView()->settings();
  settings->setDoubleTapToZoomEnabled(true); // XXX: Make this configurable

  std::string form_factor =
      base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
        switches::kFormFactor);
  if (form_factor == switches::kFormFactorTablet ||
      form_factor == switches::kFormFactorPhone) {
    settings->setAllowCustomScrollbarInMainFrame(false);
    settings->setUseWideViewport(true);
    settings->setMainFrameClipsContent(false);
    settings->setShrinksViewportContentToFit(true);
    settings->setUseMobileViewportStyle(true);

    // XXX(chrisccoulson): This should be set when the layout viewport provides
    // scrollbars (desktop), but basing this on the form-factor may not be the
    // right way. It looks like blink hides the layout scrollbars when viewport
    // meta is enabled
    settings->setHidePinchScrollbarsNearMinScale(false);
  }
}

std::string ContentRendererClient::GetUserAgentOverrideForURL(
    const GURL& url) {
  if (url.scheme() == content::kChromeUIScheme) {
    return content::BuildUserAgentFromProduct(
        base::StringPrintf("Chrome/%s", CHROME_VERSION_STRING));
  }

  return user_agent_settings_->GetUserAgentOverrideForURL(url);
}

#if defined(ENABLE_MEDIAHUB)
blink::WebMediaPlayer* ContentRendererClient::OverrideWebMediaPlayer(
              blink::WebFrame* frame,
              blink::WebMediaPlayerClient* client,
              base::WeakPtr<media::WebMediaPlayerDelegate> delegate,
              media::MediaLog* media_log) {

  RendererMediaPlayerManager* rmpm =
        RendererMediaPlayerManager::Get(
          content::RenderFrame::FromWebFrame(frame));
  DCHECK(rmpm);

  const base::CommandLine& command_line = *base::CommandLine::ForCurrentProcess();
  if (command_line.HasSwitch(switches::kEnableMediaHubAudio)) {
    return new WebMediaPlayer(frame, client, delegate, rmpm, media_log);
  }

  return nullptr;
}
#endif

void ContentRendererClient::OverrideCompositorSettings(
    cc::LayerTreeSettings* settings) {
  // XXX: If we support overlay scrollbars on desktop, then we'll want to
  //  use THINNING here, change the other settings and modify Blink to
  //  be able to have 2 different overlay scrollbar styles (a small
  //  non-hit-tested style for mobile and a much larger hit-tested style
  //  for desktop)
  if (ui::IsOverlayScrollbarEnabled()) {
    // XXX: This will need updating if we support overlay scrollbars on
    //  desktop. See https://launchpad.net/bugs/1426567
    settings->scrollbar_animator = cc::LayerTreeSettings::LINEAR_FADE;
    settings->scrollbar_fade_delay_ms = 300;
    settings->scrollbar_fade_resize_delay_ms = 2000;
    settings->scrollbar_fade_duration_ms = 300;
  }

  settings->use_external_begin_frame_source = false;
}

ContentRendererClient::ContentRendererClient() {}

ContentRendererClient::~ContentRendererClient() {}

} // namespace oxide
