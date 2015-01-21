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

#ifndef _OXIDE_SHARED_RENDERER_CONTENT_RENDERER_CLIENT_H_
#define _OXIDE_SHARED_RENDERER_CONTENT_RENDERER_CLIENT_H_

#include "base/macros.h"
#include "content/public/renderer/content_renderer_client.h"

namespace base {
template <typename Type> struct DefaultLazyInstanceTraits;
}

namespace oxide {

#if defined(ENABLE_MEDIAHUB)
class RendererMediaPlayerManager;
#endif


class ContentRendererClient final : public content::ContentRendererClient {
 public:
  // XXX(chrisccoulson): Try not to add anything here

 private:
  friend struct base::DefaultLazyInstanceTraits<ContentRendererClient>;

  ContentRendererClient();
  ~ContentRendererClient();

  // content::ContentRendererClient implementation
  void RenderThreadStarted() final;
  void RenderFrameCreated(content::RenderFrame* render_frame) final;
  void RenderViewCreated(content::RenderView* render_view) final;
  void DidCreateScriptContext(blink::WebFrame* frame,
                              v8::Handle<v8::Context> context,
                              int extension_group,
                              int world_id) final;
  std::string GetUserAgentOverrideForURL(const GURL& url) final;

#if defined(ENABLE_MEDIAHUB)
  blink::WebMediaPlayer* OverrideWebMediaPlayer(
              blink::WebFrame* frame,
              blink::WebMediaPlayerClient* client,
              base::WeakPtr<media::WebMediaPlayerDelegate> delegate,
              media::MediaLog* media_log);
#endif

  DISALLOW_COPY_AND_ASSIGN(ContentRendererClient);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_CONTENT_RENDERER_CLIENT_H_
