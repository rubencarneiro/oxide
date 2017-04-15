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
#include "base/time/time.h"
#include "cc/trees/layer_tree_settings.h"
#include "content/public/common/url_constants.h"
#include "content/public/common/user_agent.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "net/base/net_module.h"
#include "ppapi/features/features.h"
#include "third_party/WebKit/public/platform/WebURLResponse.h"
#include "third_party/WebKit/public/web/WebRuntimeFeatures.h"
#include "ui/native_theme/native_theme_switches.h"

#include "shared/common/chrome_version.h"
#include "shared/common/oxide_constants.h"
#include "shared/common/oxide_net_resource_provider.h"

#include "browser_controls_handler.h"
#include "oxide_renderer_user_agent_settings.h"
#include "oxide_script_message_dispatcher_renderer.h"
#include "oxide_user_script_scheduler.h"
#include "oxide_user_script_slave.h"
#include "oxide_web_content_settings_client.h"

#if BUILDFLAG(ENABLE_PLUGINS)
#include "pepper/oxide_pepper_render_frame_observer.h"
#endif

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
  blink::WebRuntimeFeatures::EnableOrientationEvent(true);
  // Oxide has no mechanism to display page popups
  blink::WebRuntimeFeatures::EnablePagePopup(false);
  // Oxide does not support NavigatorContentUtils.
  // See https://launchpad.net/bugs/1214046
  blink::WebRuntimeFeatures::EnableNavigatorContentUtils(false);
}

void ContentRendererClient::RenderFrameCreated(
    content::RenderFrame* render_frame) {
  new ScriptMessageDispatcherRenderer(render_frame);
  new WebContentSettingsClient(render_frame);
#if BUILDFLAG(ENABLE_PLUGINS)
  new PepperRenderFrameObserver(render_frame);
#endif
#if defined(ENABLE_MEDIAHUB)
  new RendererMediaPlayerManager(render_frame);
#endif
}

void ContentRendererClient::RenderViewCreated(
    content::RenderView* render_view) {
  new BrowserControlsHandler(render_view);
  // XXX: This is currently here because RenderFrame proxies the
  //      notifications we're interested in to RenderView. Make this
  //      a RenderFrameObserver when it grows the features we need
  new UserScriptScheduler(render_view);
}

void ContentRendererClient::AddImageContextMenuProperties(
    const blink::WebURLResponse& response,
    std::map<std::string, std::string>* properties) {
  // XXX(oSoMoN): see comment in
  // oxide::ResourceDispatcherHostDelegate::DispatchDownloadRequest(…).
  (*properties)[oxide::kImageContextMenuPropertiesMimeType] =
      response.MimeType().Utf8();
}

void ContentRendererClient::RunScriptsAtDocumentStart(
    content::RenderFrame* render_frame) {
  if (!render_frame->GetWebFrame()) {
    return;
  }

  UserScriptSlave::GetInstance()->InjectScripts(render_frame->GetWebFrame(),
                                                UserScript::DOCUMENT_START);
}

void ContentRendererClient::RunScriptsAtDocumentEnd(
    content::RenderFrame* render_frame) {
  UserScriptSlave::GetInstance()->InjectScripts(render_frame->GetWebFrame(),
                                                UserScript::DOCUMENT_END);
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
    settings->scrollbar_animator = cc::LayerTreeSettings::ANDROID_OVERLAY;
    settings->scrollbar_fade_out_delay = base::TimeDelta::FromMilliseconds(300);
    settings->scrollbar_fade_out_resize_delay = base::TimeDelta::FromSeconds(2);
    settings->scrollbar_fade_out_duration =
        base::TimeDelta::FromMilliseconds(300);
    settings->solid_color_scrollbar_color = SkColorSetARGB(128, 128, 128, 128);
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

ContentRendererClient::ContentRendererClient() {}

ContentRendererClient::~ContentRendererClient() {}

} // namespace oxide
