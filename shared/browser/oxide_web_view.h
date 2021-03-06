// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_WEB_VIEW_H_
#define _OXIDE_SHARED_BROWSER_WEB_VIEW_H_

#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string16.h"
#include "cc/output/compositor_frame_metadata.h"
#include "components/sessions/core/serialized_navigation_entry.h"
#include "content/public/browser/restore_type.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "third_party/WebKit/public/web/WebContextMenuData.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"

#include "shared/browser/clipboard/oxide_clipboard_observer.h"
#include "shared/browser/oxide_content_types.h"
#include "shared/browser/oxide_script_message_target.h"
#include "shared/browser/oxide_web_contents_view.h"
#include "shared/browser/web_contents_unique_ptr.h"
#include "shared/common/oxide_message_enums.h"
#include "shared/common/oxide_shared_export.h"

class GURL;

namespace content {

struct OpenURLParams;
class RenderFrameHost;
class RenderViewHost;
class RenderWidgetHost;
class WebContents;

} // namespace content

namespace gfx {
class Range;
}

namespace oxide {

class BrowserContext;
class CompositorFrameData;
class FilePicker;
class JavaScriptDialogFactory;
class RenderWidgetHostView;
class WebContentsClient;
class WebContentsHelper;
class WebContentsViewClient;
class WebFrame;
class WebView;
class WebViewClient;

// This is the main webview class. Implementations should customize this by
// providing an implementation of WebViewClient
class OXIDE_SHARED_EXPORT WebView : public ScriptMessageTarget,
                                    private content::WebContentsDelegate,
                                    private content::WebContentsObserver,
                                    private ClipboardObserver {
 public:

  struct CommonParams {
    CommonParams();
    ~CommonParams();

    WebViewClient* client = nullptr;
    WebContentsClient* contents_client = nullptr;
    WebContentsViewClient* view_client = nullptr;
  };

  struct CreateParams {
    CreateParams();
    ~CreateParams();

    BrowserContext* context;
    bool incognito;
    std::vector<sessions::SerializedNavigationEntry> restore_entries;
    content::RestoreType restore_type;
    int restore_index;
    gfx::Size initial_size;
    bool initially_hidden;
  };

  WebView(const CommonParams& common_params,
          const CreateParams& create_params);
  WebView(const CommonParams& common_params,
          WebContentsUniquePtr contents);

  ~WebView() override;

  WebViewClient* client() const { return client_; }

  void SetJavaScriptDialogFactory(JavaScriptDialogFactory* factory);

  // DEPRECATED. If you need to map a WebContents to WebView, then your code
  // belongs in a separate helper class
  static WebView* FromWebContents(const content::WebContents* web_contents);
  static WebView* FromRenderViewHost(content::RenderViewHost* rvh);
  static WebView* FromRenderFrameHost(content::RenderFrameHost* rfh);

  base::WeakPtr<WebView> AsWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

  const GURL& GetURL() const;
  void SetURL(const GURL& url);

  std::vector<sessions::SerializedNavigationEntry> GetState() const;

  void LoadData(const std::string& encoded_data,
                const std::string& mime_type,
                const GURL& base_url);

  std::string GetTitle() const;

  const GURL& GetFaviconURL() const;

  bool CanGoBack() const;
  bool CanGoForward() const;

  void GoBack();
  void GoForward();
  void Stop();
  void Reload();

  bool IsIncognito() const;

  bool IsLoading() const;

  BrowserContext* GetBrowserContext() const;
  content::WebContents* GetWebContents() const;
  WebContentsHelper* GetWebContentsHelper() const;

  WebFrame* GetRootFrame() const;

  const cc::CompositorFrameMetadata& compositor_frame_metadata() const {
    return compositor_frame_metadata_;
  }

  gfx::Point GetCompositorFrameScrollOffset();
  gfx::Size GetCompositorFrameContentSize();
  gfx::Size GetCompositorFrameViewportSize();

  ContentType blocked_content() const { return blocked_content_; }

  void SetCanTemporarilyDisplayInsecureContent(bool allow);
  void SetCanTemporarilyRunInsecureContent(bool allow);

  void PrepareToClose();

  const GURL& target_url() const { return target_url_; }

  blink::WebContextMenuData::EditFlags GetEditFlags() const;

  void TerminateWebProcess();

 private:
  WebView(WebViewClient* client);

  void CommonInit(WebContentsUniquePtr contents,
                  WebContentsViewClient* view_client,
                  WebContentsClient* contents_client);

  RenderWidgetHostView* GetRenderWidgetHostView() const;
  content::RenderWidgetHost* GetRenderWidgetHost() const;

  gfx::Size GetViewSizeDip() const;
  gfx::Rect GetViewBoundsDip() const;

  bool IsFullscreen() const;

  void DispatchLoadFailed(const GURL& validated_url,
                          int error_code,
                          const base::string16& error_description,
                          bool is_provisional_load = false);

  void OnDidBlockDisplayingInsecureContent();
  void OnDidBlockRunningInsecureContent();

  void DispatchPrepareToCloseResponse(bool proceed);

  void MaybeCancelFullscreenMode();

  void OnSwapCompositorFrame(const CompositorFrameData* data,
                             const cc::CompositorFrameMetadata& metadata);
  void EditingCapabilitiesChanged();

  // ScriptMessageTarget implementation
  virtual size_t GetScriptMessageHandlerCount() const override;
  virtual const ScriptMessageHandler* GetScriptMessageHandlerAt(
      size_t index) const override;

  // content::WebContentsDelegate implementation
  content::WebContents* OpenURLFromTab(
      content::WebContents* source,
      const content::OpenURLParams& params) override;
  void NavigationStateChanged(content::WebContents* source,
                              content::InvalidateTypes changed_flags) override;
  void VisibleSecurityStateChanged(content::WebContents* source) override;
  bool ShouldCreateWebContents(
      content::WebContents* source,
      content::SiteInstance* source_site_instance,
      int32_t route_id,
      int32_t main_frame_route_id,
      int32_t main_frame_widget_route_id,
      content::mojom::WindowContainerType window_container_type,
      const GURL& opener_url,
      const std::string& frame_name,
      const GURL& target_url,
      const std::string& partition_id,
      content::SessionStorageNamespace* session_storage_namespace,
      WindowOpenDisposition disposition,
      bool user_gesture) override;
  void HandleKeyboardEvent(
      content::WebContents* source,
      const content::NativeWebKeyboardEvent& event) override;
  void WebContentsCreated(content::WebContents* source,
                          int opener_render_process_id,
                          int opener_render_frame_id,
                          const std::string& frame_name,
                          const GURL& target_url,
                          content::WebContents* new_contents) override;
  void RendererUnresponsive(
      content::WebContents* source,
      const content::WebContentsUnresponsiveState& unresponsive_state) override;
  void RendererResponsive(content::WebContents* source) override;
  void AddNewContents(content::WebContents* source,
                      content::WebContents* new_contents,
                      WindowOpenDisposition disposition,
                      const gfx::Rect& initial_pos,
                      bool user_gesture,
                      bool* was_blocked) override;
  void LoadProgressChanged(content::WebContents* source,
                           double progress) override;
  void CloseContents(content::WebContents* source) override;
  void UpdateTargetURL(content::WebContents* source, const GURL& url) override;
  bool DidAddMessageToConsole(content::WebContents* source,
                              int32_t level,
                              const base::string16& message,
                              int32_t line_no,
                              const base::string16& source_id) override;
  void BeforeUnloadFired(content::WebContents* source,
                         bool proceed,
                         bool* proceed_to_fire_unload) override;
  content::JavaScriptDialogManager* GetJavaScriptDialogManager(
      content::WebContents* source) override;
  void RunFileChooser(content::RenderFrameHost* render_frame_host,
                      const content::FileChooserParams& params) override;
  bool EmbedsFullscreenWidget() const override;
  void EnterFullscreenModeForTab(content::WebContents* source,
                                 const GURL& origin) override;
  void ExitFullscreenModeForTab(content::WebContents* source) override;
  bool IsFullscreenForTabOrPending(
      const content::WebContents* source) const override;
  void FindReply(content::WebContents* source,
                 int request_id,
                 int number_of_matches,
                 const gfx::Rect& selection_rect,
                 int active_match_ordinal,
                 bool final_update) override;
  void RequestMediaAccessPermission(
      content::WebContents* source,
      const content::MediaStreamRequest& request,
      const content::MediaResponseCallback& callback) override;
  bool CheckMediaAccessPermission(content::WebContents* source,
                                  const GURL& security_origin,
                                  content::MediaStreamType type) override;

  // content::WebContentsObserver implementation
  void DidStartLoading() override;
  void DidStopLoading() override;
  void DidFinishLoad(content::RenderFrameHost* render_frame_host,
                     const GURL& validated_url) override;
  void DidFailLoad(content::RenderFrameHost* render_frame_host,
                   const GURL& validated_url,
                   int error_code,
                   const base::string16& error_description,
                   bool was_ignored_by_handler) override;
  void DidStartProvisionalLoadForFrame(
      content::RenderFrameHost* render_frame_host,
      const GURL& validated_url,
      bool is_error_page) override;
  void DidCommitProvisionalLoadForFrame(
      content::RenderFrameHost* render_frame_host,
      const GURL& url,
      ui::PageTransition transition_type) override;
  void DidFailProvisionalLoad(
      content::RenderFrameHost* render_frame_host,
      const GURL& validated_url,
      int error_code,
      const base::string16& error_description,
      bool was_ignored_by_handler) override;
  void DidNavigateMainFrame(
      const content::LoadCommittedDetails& details,
      const content::FrameNavigateParams& params) override;
  void DidGetRedirectForResourceRequest(
      const content::ResourceRedirectDetails& details) override;
  void DidShowFullscreenWidget() override;
  bool OnMessageReceived(const IPC::Message& msg,
                         content::RenderFrameHost* render_frame_host) override;

  // ClipboardObserver implementation
  void ClipboardDataChanged(ui::ClipboardType type) override;

  WebViewClient* client_;

  WebContentsUniquePtr web_contents_;

  base::WeakPtr<FilePicker> active_file_picker_;

  ContentType blocked_content_;

  std::unique_ptr<WebContentsView::SwapCompositorFrameSubscription>
      swap_compositor_frame_subscription_;
  cc::CompositorFrameMetadata compositor_frame_metadata_;

  GURL target_url_;

  blink::WebContextMenuData::EditFlags edit_flags_;

  base::WeakPtrFactory<WebView> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(WebView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_VIEW_H_
