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

#ifndef _OXIDE_SHARED_BROWSER_CHROME_CONTROLLER_H_
#define _OXIDE_SHARED_BROWSER_CHROME_CONTROLLER_H_

#include <memory>

#include "base/macros.h"
#include "base/optional.h"
#include "cc/output/compositor_frame_metadata.h"
#include "content/public/browser/web_contents_observer.h"
#include "third_party/WebKit/public/platform/WebBrowserControlsState.h"

#include "shared/browser/ssl/oxide_security_status.h"
#include "shared/browser/web_contents_data_tracker.h"
#include "shared/browser/web_process_status_monitor.h"
#include "shared/common/oxide_shared_export.h"

namespace content {
class RenderFrameHost;
class RenderWidgetHost;
class WebContents;
}

namespace oxide {

class ChromeControllerClient;
class RenderWidgetHostView;

// A mechanism to allow Oxide to position an application's UI, using the
// renderer compositor cc::BrowserControlsOffsetManager
class OXIDE_SHARED_EXPORT ChromeController
    : public WebContentsDataTracker<ChromeController>,
      public content::WebContentsObserver {
 public:
  static ChromeController* FromWebContents(content::WebContents* contents);

  ~ChromeController() override;

  float top_controls_height() const { return top_controls_height_; }
  void SetTopControlsHeight(float height);

  blink::WebBrowserControlsState constraints() const { return constraints_; }
  void SetConstraints(blink::WebBrowserControlsState constraints);

  bool animation_enabled() const { return animation_enabled_; }
  void set_animation_enabled(bool animated) { animation_enabled_ = animated; }

  void Show(bool animate);
  void Hide(bool animate);

  float GetTopControlsOffset() const;
  float GetTopContentOffset() const;

  void set_client(ChromeControllerClient* client) { client_ = client; }

  void FrameMetadataUpdated(
      const base::Optional<cc::CompositorFrameMetadata>& metadata);

 private:
  friend class WebContentsDataTracker<ChromeController>;
  ChromeController(content::WebContents* contents);

  void InitializeForHost(content::RenderFrameHost* render_frame_host,
                         bool initial_host);
  void RefreshBrowserControlsState();
  void UpdateBrowserControlsState(content::RenderFrameHost* render_frame_host,
                                  blink::WebBrowserControlsState current_state,
                                  bool animated);
  RenderWidgetHostView* GetRenderWidgetHostView();
  content::RenderWidgetHost* GetRenderWidgetHost();

  bool RendererIsUnresponsive() const;

  cc::CompositorFrameMetadata FallbackMetadata() const;

  bool CanHideBrowserControls() const;
  bool CanShowBrowserControls() const;

  void OnSecurityStatusChanged(SecurityStatus::ChangedFlags flags);
  void OnWebProcessStatusChanged();

  // content::WebContentsObserver implementation
  void RenderFrameForInterstitialPageCreated(
      content::RenderFrameHost* render_frame_host) override;
  void RenderViewHostChanged(content::RenderViewHost* old_host,
                             content::RenderViewHost* new_host) override;
  void DidCommitProvisionalLoadForFrame(
      content::RenderFrameHost* render_frame_host,
      const GURL& url,
      ui::PageTransition transition_type) override;
  void WebContentsDestroyed() override;
  void DidShowFullscreenWidget() override;
  void DidDestroyFullscreenWidget() override;
  void DidToggleFullscreenModeForTab(bool entered_fullscreen,
                                     bool will_cause_resize) override;
  void DidAttachInterstitialPage() override;
  void DidDetachInterstitialPage() override;

  ChromeControllerClient* client_;

  float top_controls_height_;
  blink::WebBrowserControlsState constraints_;
  bool animation_enabled_;

  cc::CompositorFrameMetadata current_frame_metadata_;

  std::unique_ptr<SecurityStatus::Subscription> security_status_subscription_;
  std::unique_ptr<WebProcessStatusMonitor::Subscription>
      web_process_status_subscription_;

  DISALLOW_COPY_AND_ASSIGN(ChromeController);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_CHROME_CONTROLLER_H_
