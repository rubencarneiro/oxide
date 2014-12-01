// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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
#include "content/public/common/url_utils.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "third_party/WebKit/public/web/WebSettings.h"
#include "third_party/WebKit/public/web/WebView.h"

#include "shared/common/oxide_constants.h"
#include "shared/common/oxide_messages.h"

#include "oxide_render_process_observer.h"
#include "oxide_script_message_dispatcher_renderer.h"
#include "oxide_user_script_scheduler.h"
#include "oxide_user_script_slave.h"
#include "oxide_web_permission_client.h"
#if defined(ENABLE_MEDIAHUB)
#include "media/oxide_renderer_media_player_manager.h"
#include "media/oxide_web_media_player.h"
#endif

namespace oxide {

ContentRendererClient::ContentRendererClient() {}

ContentRendererClient::~ContentRendererClient() {}

void ContentRendererClient::RenderThreadStarted() {
  new RenderProcessObserver();
  new UserScriptSlave();
}

void ContentRendererClient::RenderFrameCreated(
    content::RenderFrame* render_frame) {
  new ScriptMessageDispatcherRenderer(render_frame);
  new WebPermissionClient(render_frame);
#if defined(ENABLE_MEDIAHUB)
  new RendererMediaPlayerManager(render_frame);
#endif
}

void ContentRendererClient::RenderViewCreated(
    content::RenderView* render_view) {
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
    settings->setUseWideViewport(true);
    settings->setMainFrameClipsContent(false);
    settings->setShrinksViewportContentToFit(true);
  }
}

void ContentRendererClient::DidCreateScriptContext(
    blink::WebFrame* frame,
    v8::Handle<v8::Context> context,
    int extension_group,
    int world_id) {
  ScriptMessageDispatcherRenderer::FromWebFrame(
      frame)->DidCreateScriptContext(context, world_id);
}

std::string ContentRendererClient::GetUserAgentOverrideForURL(
    const GURL& url) {
  GURL u = url;

  // Strip username / password / fragment identifier if they exist
  if (u.has_password() || u.has_username() || u.has_ref()) {
    GURL::Replacements rep;
    rep.ClearUsername();
    rep.ClearPassword();
    rep.ClearRef();
    u = u.ReplaceComponents(rep);
  }

  // Strip query if we are above the max number of chars
  if (u.spec().size() > content::GetMaxURLChars() &&
      u.has_query()) {
    GURL::Replacements rep;
    rep.ClearQuery();
    u = u.ReplaceComponents(rep);
  }

  // If we are still over, just send the origin
  if (u.spec().size() > content::GetMaxURLChars()) {
    u = u.GetOrigin();
  }

  // Not sure we should ever hit this, but in any case - there
  // isn't much more we can do now
  if (u.spec().size() > content::GetMaxURLChars()) {
    return std::string();
  }

  bool overridden = false;
  std::string user_agent;

  content::RenderThread::Get()->Send(new OxideHostMsg_GetUserAgentOverride(
      u, &user_agent, &overridden));
  if (!overridden) {
    return std::string();
  }

  return user_agent;
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
  if (rmpm == NULL) {
    return 0;
  }

  const CommandLine& command_line = *base::CommandLine::ForCurrentProcess();
  if (command_line.HasSwitch(switches::kEnableMediaHubAudio)) {
    return new WebMediaPlayer(frame, client, delegate, rmpm, media_log);
  } else {
    return 0;
  }
}
#endif

} // namespace oxide
