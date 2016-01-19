// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_RENDERER_WEB_CONTENT_SETTINGS_CLIENT_H_
#define _OXIDE_SHARED_RENDERER_WEB_CONTENT_SETTINGS_CLIENT_H_

#include "base/compiler_specific.h"
#include "base/macros.h"

#include "content/public/renderer/render_frame_observer.h"
#include "content/public/renderer/render_frame_observer_tracker.h"
#include "third_party/WebKit/public/web/WebContentSettingsClient.h"

namespace content {
class RenderFrame;
}

namespace oxide {

class WebContentSettingsClient final :
    public content::RenderFrameObserver,
    public content::RenderFrameObserverTracker<WebContentSettingsClient>,
    public blink::WebContentSettingsClient {
 public:
  WebContentSettingsClient(content::RenderFrame* render_frame);
  ~WebContentSettingsClient();

 private:
  // content::RenderFrameObserver implementation
  void DidCommitProvisionalLoad(bool is_new_navigation,
                                bool is_same_page_navigation) final;
  bool OnMessageReceived(const IPC::Message& message) final;

  // blink::WebContentSettingsClient implementation
  bool allowDisplayingInsecureContent(bool enabled_per_settings,
                                      const blink::WebURL& url) final;
  bool allowRunningInsecureContent(bool enabled_per_settings,
                                   const blink::WebSecurityOrigin& origin,
                                   const blink::WebURL& url) final;

  void OnSetAllowDisplayingInsecureContent(bool allow);
  void OnSetAllowRunningInsecureContent(bool allow);
  void OnReloadFrame();

  bool can_display_insecure_content_;
  bool can_run_insecure_content_;

  bool did_block_displaying_insecure_content_;
  bool did_block_running_insecure_content_;

  DISALLOW_COPY_AND_ASSIGN(WebContentSettingsClient);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_WEB_CONTENT_SETTINGS_CLIENT_H_
