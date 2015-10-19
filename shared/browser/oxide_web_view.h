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

#ifndef _OXIDE_SHARED_BROWSER_WEB_VIEW_H_
#define _OXIDE_SHARED_BROWSER_WEB_VIEW_H_

#include <queue>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string16.h"
#include "cc/output/compositor_frame_metadata.h"
#include "components/sessions/core/serialized_navigation_entry.h"
#include "content/public/browser/certificate_request_result_type.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/javascript_message_type.h"
#include "content/public/common/resource_type.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"
#include "third_party/WebKit/public/platform/WebTopControlsState.h"
#include "third_party/WebKit/public/web/WebCompositionUnderline.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"

#include "shared/browser/compositor/oxide_compositor_client.h"
#include "shared/browser/oxide_certificate_error.h"
#include "shared/browser/oxide_content_types.h"
#include "shared/browser/oxide_render_widget_host_view_container.h"
#include "shared/browser/oxide_script_message_target.h"
#include "shared/browser/oxide_security_status.h"
#include "shared/browser/oxide_security_types.h"
#include "shared/browser/oxide_touch_event_state.h"
#include "shared/browser/oxide_web_preferences_observer.h"
#include "shared/common/oxide_message_enums.h"

class GURL;

namespace blink {
class WebMouseEvent;
class WebMouseWheelEvent;
} // namespace blink

namespace cc {
class SolidColorLayer;
}

namespace content {

struct ContextMenuParams;
struct MenuItem;
class NativeWebKeyboardEvent;
class NotificationRegistrar;
struct OpenURLParams;
class RenderFrameHost;
class RenderViewHost;
class RenderWidgetHostImpl;
class WebContents;
class WebContentsImpl;
class WebCursor;

} // namespace content

namespace gfx {
class Range;
class Size;
}

namespace net {
class SSLInfo;
}

namespace ui {
class TouchEvent;
}

namespace oxide {

class BrowserContext;
class Compositor;
class CompositorFrameHandle;
class FilePicker;
class JavaScriptDialog;
class ResourceDispatcherHostLoginDelegate;
class RenderWidgetHostView;
class WebContextMenu;
class WebFrame;
class WebPopupMenu;
class WebPreferences;
class WebView;
class WebViewClient;
class WebViewContentsHelper;

class WebViewIterator final {
 public:
  ~WebViewIterator();

  bool HasMore() const;
  WebView* GetNext();

 private:
  friend class WebView;

  WebViewIterator(const std::vector<WebView*>& views);

  typedef std::vector<base::WeakPtr<WebView> > Vector;
  Vector views_;
  Vector::iterator current_;
};

// This is the main webview class. Implementations should customize this by
// providing an implementation of WebViewClient
class WebView : public ScriptMessageTarget,
                private CompositorClient,
                private WebPreferencesObserver,
                private content::NotificationObserver,
                private RenderWidgetHostViewContainer,
                private content::WebContentsDelegate,
                private content::WebContentsObserver {
 public:

  struct Params {
    Params();
    ~Params();

    WebViewClient* client;
    BrowserContext* context;
    bool incognito;
    std::vector<sessions::SerializedNavigationEntry> restore_entries;
    content::NavigationController::RestoreType restore_type;
    int restore_index;
  };

  WebView(const Params& params);
  WebView(scoped_ptr<content::WebContents> contents,
          WebViewClient* client);

  ~WebView() override;

  WebViewClient* client() const { return client_; }

  static WebView* FromWebContents(const content::WebContents* web_contents);
  static WebView* FromRenderViewHost(content::RenderViewHost* rvh);
  static WebView* FromRenderFrameHost(content::RenderFrameHost* rfh);

  static WebViewIterator GetAllWebViews();

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

  bool IsFullscreen() const;
  void SetIsFullscreen(bool fullscreen);

  void WasResized();
  void ScreenUpdated();
  void VisibilityChanged();
  void FocusChanged();
  void InputPanelVisibilityChanged();
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

  gfx::Size GetViewSizePix() const;
  gfx::Rect GetViewBoundsDip() const;
  gfx::Size GetViewSizeDip() const;

  const cc::CompositorFrameMetadata& compositor_frame_metadata() const {
    return compositor_frame_metadata_;
  }
  gfx::Point GetCompositorFrameScrollOffsetPix();
  gfx::Size GetCompositorFrameContentSizePix();
  gfx::Size GetCompositorFrameViewportSizePix();

  int GetLocationBarOffsetPix();
  int GetLocationBarContentOffsetPix();
  float GetLocationBarContentOffsetDip();

  const SecurityStatus& security_status() const { return security_status_; }

  ContentType blocked_content() const { return blocked_content_; }

  float GetLocationBarHeightDip() const;
  int GetLocationBarHeightPix() const;
  void SetLocationBarHeightPix(int height);

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

  void ShowContextMenu(content::RenderFrameHost* render_frame_host,
                       const content::ContextMenuParams& params);
  void ShowPopupMenu(content::RenderFrameHost* render_frame_host,
                     const gfx::Rect& bounds,
                     int selected_item,
                     const std::vector<content::MenuItem>& items,
                     bool allow_multiple_selection);
  void HidePopupMenu();

  void AllowCertificateError(content::RenderFrameHost* rfh,
                             int cert_error,
                             const net::SSLInfo& ssl_info,
                             const GURL& request_url,
                             content::ResourceType resource_type,
                             bool overridable,
                             bool strict_enforcement,
                             const base::Callback<void(bool)>& callback,
                             content::CertificateRequestResultType* result);

  void HandleKeyEvent(const content::NativeWebKeyboardEvent& event);
  void HandleMouseEvent(const blink::WebMouseEvent& event);
  void HandleTouchEvent(const ui::TouchEvent& event);
  void HandleWheelEvent(const blink::WebMouseWheelEvent& event);

  void ImeCommitText(const base::string16& text,
                     const gfx::Range& replacement_range);
  void ImeSetComposingText(
      const base::string16& text,
      const std::vector<blink::WebCompositionUnderline>& underlines,
      const gfx::Range& selection_range);

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

  CompositorFrameHandle* GetCompositorFrameHandle() const;
  void DidCommitCompositorFrame();

  blink::WebScreenInfo GetScreenInfo() const;
  gfx::Rect GetViewBoundsPix() const;
  bool IsVisible() const;
  bool HasFocus() const;
  bool IsInputPanelVisible() const;

  JavaScriptDialog* CreateJavaScriptDialog(
      content::JavaScriptMessageType javascript_message_type);
  JavaScriptDialog* CreateBeforeUnloadDialog();

  bool ShouldHandleNavigation(const GURL& url, bool has_user_gesture);

  bool CanCreateWindows() const;

  ui::TextInputType text_input_type() const { return text_input_type_; }
  bool show_ime_if_needed() const { return show_ime_if_needed_; }
  bool focused_node_is_editable() const { return focused_node_is_editable_; }

  gfx::Rect caret_rect() const { return caret_rect_; }
  size_t selection_cursor_position() const { return selection_cursor_position_; }
  size_t selection_anchor_position() const { return selection_anchor_position_; }

  base::string16 GetSelectedText() const;
  const base::string16& GetSelectionText() const;

 private:
  WebView(WebViewClient* client);

  void CommonInit(scoped_ptr<content::WebContents> contents);

  RenderWidgetHostView* GetRenderWidgetHostView() const;
  content::RenderViewHost* GetRenderViewHost() const;
  content::RenderWidgetHostImpl* GetRenderWidgetHostImpl() const;

  void DispatchLoadFailed(const GURL& validated_url,
                          int error_code,
                          const base::string16& error_description,
                          bool is_provisional_load = false);

  void OnDidBlockDisplayingInsecureContent();
  void OnDidBlockRunningInsecureContent();

  bool ShouldScrollFocusedEditableNodeIntoView();
  void MaybeScrollFocusedEditableNodeIntoView();

  float GetFrameMetadataScaleToPix();

  void InitializeTopControlsForHost(content::RenderViewHost* rvh,
                                    bool initial_host);

  void DispatchPrepareToCloseResponse(bool proceed);

  void RestartFindInPage();

  // ScriptMessageTarget implementation
  virtual size_t GetScriptMessageHandlerCount() const override;
  virtual const ScriptMessageHandler* GetScriptMessageHandlerAt(
      size_t index) const override;

  // CompositorClient implementation
  void CompositorDidCommit() final;
  void CompositorSwapFrame(CompositorFrameHandle* handle) final;

  // WebPreferencesObserver implementation
  void WebPreferencesDestroyed() final;

  // content::NotificationObserver implementation
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) final;

  // RenderWidgetHostViewContainer implementation
  Compositor* GetCompositor() const final;
  void AttachLayer(scoped_refptr<cc::Layer> layer) final;
  void DetachLayer(scoped_refptr<cc::Layer> layer) final;
  void UpdateCursor(const content::WebCursor& cursor) final;
  void TextInputStateChanged(ui::TextInputType type,
                             bool show_ime_if_needed) final;
  void FocusedNodeChanged(bool is_editable_node) final;
  void ImeCancelComposition() final;
  void SelectionBoundsChanged(const gfx::Rect& caret_rect,
                              size_t selection_cursor_position,
                              size_t selection_anchor_position) final;
  void SelectionChanged() final;

  // content::WebContentsDelegate implementation
  content::WebContents* OpenURLFromTab(content::WebContents* source,
                                       const content::OpenURLParams& params) final;
  void NavigationStateChanged(content::WebContents* source,
                              content::InvalidateTypes changed_flags) final;
  void VisibleSSLStateChanged(const content::WebContents* source) final;
  bool ShouldCreateWebContents(
      content::WebContents* source,
      int route_id,
      int main_frame_route_id,
      WindowContainerType window_container_type,
      const std::string& frame_name,
      const GURL& target_url,
      const std::string& partition_id,
      content::SessionStorageNamespace* session_storage_namespace,
      WindowOpenDisposition disposition,
      bool user_gesture) final;
  void HandleKeyboardEvent(content::WebContents* source,
                           const content::NativeWebKeyboardEvent& event) final;
  void WebContentsCreated(content::WebContents* source,
                          int source_frame_id,
                          const std::string& frame_name,
                          const GURL& target_url,
                          content::WebContents* new_contents) final;
  void AddNewContents(content::WebContents* source,
                      content::WebContents* new_contents,
                      WindowOpenDisposition disposition,
                      const gfx::Rect& initial_pos,
                      bool user_gesture,
                      bool* was_blocked) final;
  void LoadProgressChanged(content::WebContents* source, double progress) final;
  void CloseContents(content::WebContents* source) final;
  bool AddMessageToConsole(content::WebContents* source,
               int32 level,
               const base::string16& message,
               int32 line_no,
               const base::string16& source_id) final;
  void BeforeUnloadFired(content::WebContents* source,
                         bool proceed,
                         bool* proceed_to_fire_unload) final;
  content::JavaScriptDialogManager* GetJavaScriptDialogManager(
      content::WebContents* source) final;
  void RunFileChooser(content::WebContents* web_contents,
                      const content::FileChooserParams& params) final;
  void EnterFullscreenModeForTab(content::WebContents* source,
                                 const GURL& origin) final;
  void ExitFullscreenModeForTab(content::WebContents* source) final;
  bool IsFullscreenForTabOrPending(
      const content::WebContents* source) const final;
  void FindReply(content::WebContents* source,
                 int request_id,
                 int number_of_matches,
                 const gfx::Rect& selection_rect,
                 int active_match_ordinal,
                 bool final_update) final;
  void RequestMediaAccessPermission(
      content::WebContents* source,
      const content::MediaStreamRequest& request,
      const content::MediaResponseCallback& callback) final;
  bool CheckMediaAccessPermission(content::WebContents* source,
                                  const GURL& security_origin,
                                  content::MediaStreamType type) final;

  // content::WebContentsObserver implementation
  void RenderFrameDeleted(content::RenderFrameHost* render_frame_host) final;
  void RenderFrameHostChanged(content::RenderFrameHost* old_host,
                              content::RenderFrameHost* new_host) final;
  void RenderViewReady() final;
  void RenderProcessGone(base::TerminationStatus status) final;
  void RenderViewHostChanged(content::RenderViewHost* old_host,
                             content::RenderViewHost* new_host) final;
  void DidStartProvisionalLoadForFrame(
      content::RenderFrameHost* render_frame_host,
      const GURL& validated_url,
      bool is_error_page,
      bool is_iframe_srcdoc) final;
  void DidCommitProvisionalLoadForFrame(
      content::RenderFrameHost* render_frame_host,
      const GURL& url,
      ui::PageTransition transition_type) final;
  void DidFailProvisionalLoad(
      content::RenderFrameHost* render_frame_host,
      const GURL& validated_url,
      int error_code,
      const base::string16& error_description,
      bool was_ignored_by_handler) final;
  void DidNavigateMainFrame(
      const content::LoadCommittedDetails& details,
      const content::FrameNavigateParams& params) final;
  void DidNavigateAnyFrame(
      content::RenderFrameHost* render_frame_host,
      const content::LoadCommittedDetails& details,
      const content::FrameNavigateParams& params) final;
  void DidFinishLoad(content::RenderFrameHost* render_frame_host,
                     const GURL& validated_url) final;
  void DidFailLoad(content::RenderFrameHost* render_frame_host,
                   const GURL& validated_url,
                   int error_code,
                   const base::string16& error_description,
                   bool was_ignored_by_handler) final;
  void DidGetRedirectForResourceRequest(
      content::RenderFrameHost* render_frame_host,
      const content::ResourceRedirectDetails& details) final;
  void NavigationEntryCommitted(
      const content::LoadCommittedDetails& load_details) final;
  void DidStartLoading() final;
  void DidStopLoading() final;
  void TitleWasSet(content::NavigationEntry* entry, bool explicit_set) final;
  bool OnMessageReceived(const IPC::Message& msg,
                         content::RenderFrameHost* render_frame_host) final;

  WebViewClient* client_;

  ui::TextInputType text_input_type_;
  bool show_ime_if_needed_;
  bool focused_node_is_editable_;

  gfx::Rect caret_rect_;
  size_t selection_cursor_position_;
  size_t selection_anchor_position_;

  struct WebContentsDeleter {
    void operator()(content::WebContents* contents);
  };
  typedef scoped_ptr<content::WebContents, WebContentsDeleter>
      WebContentsScopedPtr;

  WebContentsScopedPtr web_contents_;
  WebViewContentsHelper* web_contents_helper_;

  scoped_ptr<Compositor> compositor_;
  scoped_refptr<cc::SolidColorLayer> root_layer_;

  scoped_refptr<CompositorFrameHandle> current_compositor_frame_;
  std::vector<scoped_refptr<CompositorFrameHandle> > previous_compositor_frames_;
  std::queue<uint32> received_surface_ids_;

  gfx::Point global_mouse_position_;
  TouchEventState touch_state_;

  content::NotificationRegistrar registrar_;

  bool is_fullscreen_;
  base::WeakPtr<WebPopupMenu> active_popup_menu_;
  base::WeakPtr<FilePicker> active_file_picker_;

  CertificateErrorManager certificate_error_manager_;

  ContentType blocked_content_;

  cc::CompositorFrameMetadata pending_compositor_frame_metadata_;
  cc::CompositorFrameMetadata compositor_frame_metadata_;

  SecurityStatus security_status_;

  int location_bar_height_pix_;
  blink::WebTopControlsState location_bar_constraints_;
  bool location_bar_animated_;

  base::WeakPtrFactory<WebView> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(WebView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_VIEW_H_
