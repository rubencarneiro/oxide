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
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/restore_type.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/javascript_message_type.h"
#include "third_party/WebKit/public/platform/WebTopControlsState.h"
#include "third_party/WebKit/public/web/WebContextMenuData.h"
#include "ui/display/display.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"

#include "shared/browser/clipboard/oxide_clipboard_observer.h"
#include "shared/browser/compositor/oxide_compositor_observer.h"
#include "shared/browser/oxide_content_types.h"
#include "shared/browser/oxide_render_object_id.h"
#include "shared/browser/oxide_script_message_target.h"
#include "shared/browser/oxide_web_preferences_observer.h"
#include "shared/common/oxide_message_enums.h"
#include "shared/common/oxide_shared_export.h"

class GURL;

namespace content {

class NotificationRegistrar;
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
class FilePicker;
class JavaScriptDialog;
class ResourceDispatcherHostLoginDelegate;
class RenderWidgetHostView;
class WebContentsViewClient;
class WebFrame;
class WebPreferences;
class WebView;
class WebViewClient;
class WebContentsHelper;

// This is the main webview class. Implementations should customize this by
// providing an implementation of WebViewClient
class OXIDE_SHARED_EXPORT WebView : public ScriptMessageTarget,
                                    private CompositorObserver,
                                    private WebPreferencesObserver,
                                    private content::NotificationObserver,
                                    private content::WebContentsDelegate,
                                    private content::WebContentsObserver,
                                    private ClipboardObserver {
 public:

  struct CommonParams {
    CommonParams();
    ~CommonParams();

    WebViewClient* client;
    WebContentsViewClient* view_client;
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
          std::unique_ptr<content::WebContents> contents);

  ~WebView() override;

  WebViewClient* client() const { return client_; }

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

  void UpdateWebPreferences();

  BrowserContext* GetBrowserContext() const;
  content::WebContents* GetWebContents() const;

  int GetNavigationEntryCount() const;
  int GetNavigationCurrentEntryIndex() const;
  void SetNavigationCurrentEntryIndex(int index);
  int GetNavigationEntryUniqueID(int index) const;
  const GURL& GetNavigationEntryUrl(int index) const;
  std::string GetNavigationEntryTitle(int index) const;
  base::Time GetNavigationEntryTimestamp(int index) const;

  WebFrame* GetRootFrame() const;

  WebPreferences* GetWebPreferences();
  void SetWebPreferences(WebPreferences* prefs);

  const cc::CompositorFrameMetadata& compositor_frame_metadata() const {
    return compositor_frame_metadata_;
  }

  gfx::Point GetCompositorFrameScrollOffset();
  gfx::Size GetCompositorFrameContentSize();
  gfx::Size GetCompositorFrameViewportSize();

  float GetLocationBarOffset() const;
  float GetLocationBarContentOffset() const;

  ContentType blocked_content() const { return blocked_content_; }

  float GetLocationBarHeight() const;
  void SetLocationBarHeight(float height);

  blink::WebTopControlsState location_bar_constraints() const {
    return location_bar_constraints_;
  }
  void SetLocationBarConstraints(blink::WebTopControlsState constraints);

  bool location_bar_animated() const { return location_bar_animated_; }
  void set_location_bar_animated(bool animated) {
    location_bar_animated_ = animated;
  }

  void ShowLocationBar(bool animate);
  void HideLocationBar(bool animate);

  void SetCanTemporarilyDisplayInsecureContent(bool allow);
  void SetCanTemporarilyRunInsecureContent(bool allow);

  void PrepareToClose();

  void DownloadRequested(
      const GURL& url,
      const std::string& mime_type,
      const bool should_prompt,
      const base::string16& suggested_filename,
      const std::string& cookies,
      const std::string& referrer,
      const std::string& user_agent);

  void HttpAuthenticationRequested(
      ResourceDispatcherHostLoginDelegate* login_delegate);

  display::Display GetDisplay() const;

  JavaScriptDialog* CreateJavaScriptDialog(
      content::JavaScriptMessageType javascript_message_type);
  JavaScriptDialog* CreateBeforeUnloadDialog();

  bool ShouldHandleNavigation(const GURL& url, bool has_user_gesture);

  bool CanCreateWindows() const;

  const GURL& target_url() const { return target_url_; }

  blink::WebContextMenuData::EditFlags GetEditFlags() const;

 private:
  WebView(WebViewClient* client);

  void CommonInit(std::unique_ptr<content::WebContents> contents,
                  WebContentsViewClient* view_client);

  RenderWidgetHostView* GetRenderWidgetHostView() const;
  content::RenderViewHost* GetRenderViewHost() const;
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

  void InitializeTopControlsForHost(content::RenderViewHost* rvh,
                                    bool initial_host);

  void DispatchPrepareToCloseResponse(bool proceed);

  void MaybeCancelFullscreenMode();

  void EditingCapabilitiesChanged();

  // ScriptMessageTarget implementation
  virtual size_t GetScriptMessageHandlerCount() const override;
  virtual const ScriptMessageHandler* GetScriptMessageHandlerAt(
      size_t index) const override;


  // CompositorObserver implementation
  void CompositorWillRequestSwapFrame() override;

  // WebPreferencesObserver implementation
  void WebPreferencesDestroyed() override;

  // content::NotificationObserver implementation
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) override;

  // content::WebContentsDelegate implementation
  content::WebContents* OpenURLFromTab(
      content::WebContents* source,
      const content::OpenURLParams& params) override;
  void NavigationStateChanged(content::WebContents* source,
                              content::InvalidateTypes changed_flags) override;
  void VisibleSSLStateChanged(const content::WebContents* source) override;
  bool ShouldCreateWebContents(
      content::WebContents* source,
      int route_id,
      int main_frame_route_id,
      int main_frame_widget_route_id,
      WindowContainerType window_container_type,
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
                          int source_frame_id,
                          const std::string& frame_name,
                          const GURL& target_url,
                          content::WebContents* new_contents) override;
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
  bool AddMessageToConsole(content::WebContents* source,
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
  void RenderFrameForInterstitialPageCreated(
      content::RenderFrameHost* render_frame_host) override;
  void RenderViewReady() override;
  void RenderProcessGone(base::TerminationStatus status) override;
  void RenderViewHostChanged(content::RenderViewHost* old_host,
                             content::RenderViewHost* new_host) override;
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
      bool is_error_page,
      bool is_iframe_srcdoc) override;
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
  void NavigationEntryCommitted(
      const content::LoadCommittedDetails& load_details) override;
  void TitleWasSet(content::NavigationEntry* entry, bool explicit_set) override;
  void DidShowFullscreenWidget() override;
  bool OnMessageReceived(const IPC::Message& msg,
                         content::RenderFrameHost* render_frame_host) override;

  // ClipboardObserver implementation
  void ClipboardDataChanged(ui::ClipboardType type) override;

  WebViewClient* client_;

  struct WebContentsDeleter {
    void operator()(content::WebContents* contents);
  };
  typedef std::unique_ptr<content::WebContents, WebContentsDeleter>
      WebContentsScopedPtr;

  WebContentsScopedPtr web_contents_;
  WebContentsHelper* web_contents_helper_;

  content::NotificationRegistrar registrar_;

  base::WeakPtr<FilePicker> active_file_picker_;

  ContentType blocked_content_;

  cc::CompositorFrameMetadata compositor_frame_metadata_;

  int location_bar_height_;
  blink::WebTopControlsState location_bar_constraints_;
  bool location_bar_animated_;

  GURL target_url_;

  blink::WebContextMenuData::EditFlags edit_flags_;

  base::WeakPtrFactory<WebView> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(WebView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_VIEW_H_
