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

#ifndef _OXIDE_SHARED_RENDERER_CONTENT_RENDERER_CLIENT_H_
#define _OXIDE_SHARED_RENDERER_CONTENT_RENDERER_CLIENT_H_

#include <memory>

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/renderer/content_renderer_client.h"

namespace oxide {

#if defined(ENABLE_MEDIAHUB)
class RendererMediaPlayerManager;
#endif

class RendererUserAgentSettings;

class ContentRendererClient : public content::ContentRendererClient {
 public:
  ContentRendererClient();
  ~ContentRendererClient();

  // XXX(chrisccoulson): Try not to add anything here

 private:
  // content::ContentRendererClient implementation
  void RenderThreadStarted() override;
  void RenderFrameCreated(content::RenderFrame* render_frame) override;
  void RenderViewCreated(content::RenderView* render_view) override;
  void AddImageContextMenuProperties(
      const blink::WebURLResponse& response,
      std::map<std::string, std::string>* properties) override;
  void RunScriptsAtDocumentStart(content::RenderFrame* render_frame) override;
  void RunScriptsAtDocumentEnd(content::RenderFrame* render_frame) override;
#if defined(ENABLE_MEDIAHUB)
  blink::WebMediaPlayer* OverrideWebMediaPlayer(
      blink::WebFrame* frame,
      blink::WebMediaPlayerClient* client,
      base::WeakPtr<media::WebMediaPlayerDelegate> delegate,
      media::MediaLog* media_log) override;
#endif
  void OverrideCompositorSettings(cc::LayerTreeSettings* settings) override;
  std::string GetUserAgentOverrideForURL(const GURL& url) override;

  std::unique_ptr<RendererUserAgentSettings> user_agent_settings_;

  DISALLOW_COPY_AND_ASSIGN(ContentRendererClient);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_CONTENT_RENDERER_CLIENT_H_
