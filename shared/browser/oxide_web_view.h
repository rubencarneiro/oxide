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

#ifndef _OXIDE_SHARED_BROWSER_WEB_VIEW_H_
#define _OXIDE_SHARED_BROWSER_WEB_VIEW_H_

#include <queue>
#include <set>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string16.h"
#include "cc/output/compositor_frame_metadata.h"
#include "content/browser/renderer_host/event_with_latency_info.h"
#include "content/common/input/input_event_ack_state.h"
#include "content/public/browser/certificate_request_result_type.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/javascript_message_type.h"
#include "content/public/common/resource_type.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"
#include "third_party/WebKit/public/web/WebCompositionUnderline.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/size.h"

#include "shared/browser/compositor/oxide_compositor_client.h"
#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_content_types.h"
#include "shared/browser/oxide_gesture_provider.h"
#include "shared/browser/oxide_permission_request.h"
#include "shared/browser/oxide_script_message_target.h"
#include "shared/browser/oxide_security_status.h"
#include "shared/browser/oxide_security_types.h"
#include "shared/browser/oxide_web_preferences_observer.h"
#include "shared/browser/oxide_web_view_contents_helper_delegate.h"
#include "shared/common/oxide_message_enums.h"

class GURL;

namespace blink {
class WebMouseEvent;
class WebMouseWheelEvent;
} // namespace blink

namespace content {

class FrameTree;
class FrameTreeNode;
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
class GestureEvent;
class TouchEvent;
}

namespace oxide {

class BrowserContext;
class Compositor;
class CompositorFrameHandle;
class FilePicker;
class JavaScriptDialog;
class RenderWidgetHostView;
class WebFrame;
class WebPopupMenu;
class WebPreferences;
class WebViewContentsHelper;

// This is the main webview class. Implementations should subclass
// this. Note that this class will hold the main browser process
// components alive
class WebView : public ScriptMessageTarget,
                private CompositorClient,
                private WebPreferencesObserver,
                private GestureProviderClient,
                private content::NotificationObserver,
                private WebViewContentsHelperDelegate,
                private content::WebContentsObserver {
 public:
  virtual ~WebView();

  struct Params {
    Params() :
        context(NULL),
        incognito(false) {}

    BrowserContext* context;
    ScopedNewContentsHolder contents;
    bool incognito;
  };

  virtual void Init(Params* params);

  static WebView* FromWebContents(const content::WebContents* web_contents);
  static WebView* FromRenderViewHost(content::RenderViewHost* rvh);
  static WebView* FromRenderFrameHost(content::RenderFrameHost* rfh);

  static std::set<WebView*> GetAllWebViewsFor(
      BrowserContext * browser_context);

  const GURL& GetURL() const;
  void SetURL(const GURL& url);

  void LoadData(const std::string& encodedData,
                const std::string& mimeType,
                const GURL& baseUrl);

  std::string GetTitle() const;

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
  void VisibilityChanged();
  void FocusChanged();

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
  content::FrameTree* GetFrameTree();

  WebPreferences* GetWebPreferences();
  void SetWebPreferences(WebPreferences* prefs);

  gfx::Size GetContainerSizePix() const;
  gfx::Rect GetContainerBoundsDip() const;
  gfx::Size GetContainerSizeDip() const;

  const cc::CompositorFrameMetadata& compositor_frame_metadata() const {
    return compositor_frame_metadata_;
  }

  const SecurityStatus& security_status() const { return security_status_; }

  ContentType blocked_content() const { return blocked_content_; }

  void SetCanTemporarilyDisplayInsecureContent(bool allow);
  void SetCanTemporarilyRunInsecureContent(bool allow);

  void ShowPopupMenu(content::RenderFrameHost* render_frame_host,
                     const gfx::Rect& bounds,
                     int selected_item,
                     const std::vector<content::MenuItem>& items,
                     bool allow_multiple_selection);
  void HidePopupMenu();

  void RequestGeolocationPermission(
      const GURL& origin,
      const base::Callback<void(bool)>& callback,
      base::Closure* cancel_callback);

  void AllowCertificateError(content::RenderFrameHost* rfh,
                             int cert_error,
                             const net::SSLInfo& ssl_info,
                             const GURL& request_url,
                             content::ResourceType resource_type,
                             bool overridable,
                             bool strict_enforcement,
                             const base::Callback<void(bool)>& callback,
                             content::CertificateRequestResultType* result);
                             
  void UpdateWebPreferences();

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
      const std::string& mimeType,
      const bool shouldPrompt,
      const base::string16& suggestedFilename,
      const std::string& cookies,
      const std::string& referrer);

  Compositor* compositor() const { return compositor_.get(); }

  CompositorFrameHandle* GetCompositorFrameHandle() const;
  void DidCommitCompositorFrame();

  // Interface with RWHV ========
  void EvictCurrentFrame();
  void UpdateFrameMetadata(const cc::CompositorFrameMetadata& metadata);

  void ProcessAckedTouchEvent(bool consumed);

  virtual void UpdateCursor(const content::WebCursor& cursor);
  void TextInputStateChanged(ui::TextInputType type,
                             bool show_ime_if_needed);
  void FocusedNodeChanged(bool is_editable_node);
  virtual void ImeCancelComposition();
  void SelectionBoundsChanged(const gfx::Rect& caret_rect,
                              size_t selection_cursor_position,
                              size_t selection_anchor_position);
  virtual void SelectionChanged();
  // ============================

  virtual blink::WebScreenInfo GetScreenInfo() const = 0;
  virtual gfx::Rect GetContainerBoundsPix() const = 0;
  virtual bool IsVisible() const = 0;
  virtual bool HasFocus() const = 0;

  virtual JavaScriptDialog* CreateJavaScriptDialog(
      content::JavaScriptMessageType javascript_message_type,
      bool* did_suppress_message);
  virtual JavaScriptDialog* CreateBeforeUnloadDialog();

  virtual void FrameAdded(WebFrame* frame);
  virtual void FrameRemoved(WebFrame* frame);

  virtual bool CanCreateWindows() const;

 protected:
  WebView();

  base::string16 GetSelectedText() const;
  const base::string16& GetSelectionText() const;

  ui::TextInputType text_input_type_;
  bool show_ime_if_needed_;
  bool focused_node_is_editable_;

  gfx::Rect caret_rect_;
  size_t selection_cursor_position_;
  size_t selection_anchor_position_;

 private:
  RenderWidgetHostView* GetRenderWidgetHostView() const;
  content::RenderViewHost* GetRenderViewHost() const;
  content::RenderWidgetHostImpl* GetRenderWidgetHostImpl() const;

  void DispatchLoadFailed(const GURL& validated_url,
                          int error_code,
                          const base::string16& error_description);

  void OnDidBlockDisplayingInsecureContent();
  void OnDidBlockRunningInsecureContent();

  // ScriptMessageTarget implementation
  virtual size_t GetScriptMessageHandlerCount() const OVERRIDE;
  virtual ScriptMessageHandler* GetScriptMessageHandlerAt(
      size_t index) const OVERRIDE;

  // CompositorClient implementation
  void CompositorDidCommit() FINAL;
  void CompositorSwapFrame(uint32 surface_id,
                           CompositorFrameHandle* frame) FINAL;

  // WebPreferencesObserver implementation
  void WebPreferencesDestroyed() FINAL;

  // GestureProviderClient implementation
  void OnGestureEvent(const blink::WebGestureEvent& event) FINAL;

  // content::NotificationObserver implementation
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) FINAL;

  // WebViewContentsHelperDelegate implementation
  content::WebContents* OpenURL(const content::OpenURLParams& params) FINAL;
  void NavigationStateChanged(content::InvalidateTypes flags) FINAL;
  void SSLStateChanged() FINAL;
  bool ShouldCreateWebContents(const GURL& target_url,
                               WindowOpenDisposition disposition,
                               bool user_gesture) FINAL;
  bool CreateNewViewAndAdoptWebContents(
      ScopedNewContentsHolder contents,
      WindowOpenDisposition disposition,
      const gfx::Rect& initial_pos) FINAL;
  void LoadProgressChanged(double progress) FINAL;
  void AddMessageToConsole(int32 level,
                           const base::string16& message,
                           int32 line_no,
                           const base::string16& source_id) FINAL;
  bool RunFileChooser(const content::FileChooserParams& params) FINAL;
  void ToggleFullscreenMode(bool enter) FINAL;
  void NotifyWebPreferencesDestroyed() FINAL;
  void HandleUnhandledKeyboardEvent(
      const content::NativeWebKeyboardEvent& event) FINAL;

  // content::WebContentsObserver implementation
  void RenderFrameCreated(content::RenderFrameHost* render_frame_host) FINAL;

  void RenderProcessGone(base::TerminationStatus status) FINAL;
  void RenderViewHostChanged(content::RenderViewHost* old_host,
                             content::RenderViewHost* new_host) FINAL;

  void DidStartProvisionalLoadForFrame(
      content::RenderFrameHost* render_frame_host,
      const GURL& validated_url,
      bool is_error_page,
      bool is_iframe_srcdoc) FINAL;

  void DidCommitProvisionalLoadForFrame(
      content::RenderFrameHost* render_frame_host,
      const GURL& url,
      content::PageTransition transition_type) FINAL;

  void DidFailProvisionalLoad(
      content::RenderFrameHost* render_frame_host,
      const GURL& validated_url,
      int error_code,
      const base::string16& error_description) FINAL;

  void DidNavigateMainFrame(
      const content::LoadCommittedDetails& details,
      const content::FrameNavigateParams& params) FINAL;

  void DidFinishLoad(content::RenderFrameHost* render_frame_host,
                     const GURL& validated_url) FINAL;
  void DidFailLoad(content::RenderFrameHost* render_frame_host,
                   const GURL& validated_url,
                   int error_code,
                   const base::string16& error_description) FINAL;

  void NavigationEntryCommitted(
      const content::LoadCommittedDetails& load_details) FINAL;

  void FrameDetached(content::RenderFrameHost* render_frame_host) FINAL;

  void TitleWasSet(content::NavigationEntry* entry, bool explicit_set) FINAL;

  void DidUpdateFaviconURL(
      const std::vector<content::FaviconURL>& candidates) FINAL;

  bool OnMessageReceived(const IPC::Message& msg,
                         content::RenderFrameHost* render_frame_host) FINAL;

  // Override in sub-classes
  virtual void OnURLChanged();
  virtual void OnTitleChanged();
  virtual void OnIconChanged(const GURL& icon);
  virtual void OnCommandsUpdated();

  virtual void OnLoadProgressChanged(double progress);

  virtual void OnLoadStarted(const GURL& validated_url,
                             bool is_error_frame);
  virtual void OnLoadStopped(const GURL& validated_url);
  virtual void OnLoadFailed(const GURL& validated_url,
                            int error_code,
                            const std::string& error_description);
  virtual void OnLoadSucceeded(const GURL& validated_url);

  virtual void OnNavigationEntryCommitted();
  virtual void OnNavigationListPruned(bool from_front, int count);
  virtual void OnNavigationEntryChanged(int index);

  virtual bool OnAddMessageToConsole(int32 level,
                                     const base::string16& message,
                                     int32 line_no,
                                     const base::string16& source_id);

  virtual void OnToggleFullscreenMode(bool enter);

  virtual void OnWebPreferencesDestroyed();

  virtual void OnRequestGeolocationPermission(
      const GURL& origin,
      const GURL& embedder,
      scoped_ptr<SimplePermissionRequest> request);

  virtual void OnUnhandledKeyboardEvent(
      const content::NativeWebKeyboardEvent& event);

  virtual void OnFrameMetadataUpdated(const cc::CompositorFrameMetadata& old);

  virtual void OnDownloadRequested(
      const GURL& url,
      const std::string& mimeType,
      const bool shouldPrompt,
      const base::string16& suggestedFilename,
      const std::string& cookies,
      const std::string& referrer);

  virtual bool ShouldHandleNavigation(const GURL& url,
                                      WindowOpenDisposition disposition,
                                      bool user_gesture);

  virtual void OnUrlRedirection(
      const GURL& url,
      const GURL& original_url,
      const std::string& referrer,
      const std::string& method,
      bool isMainFrame,
      int http_response_code);

  virtual WebFrame* CreateWebFrame(content::FrameTreeNode* node) = 0;
  virtual WebPopupMenu* CreatePopupMenu(content::RenderFrameHost* rfh);

  virtual WebView* CreateNewWebView(const gfx::Rect& initial_pos,
                                    WindowOpenDisposition disposition);

  virtual FilePicker* CreateFilePicker(content::RenderViewHost* rvh);

  virtual void OnSwapCompositorFrame() = 0;
  virtual void OnEvictCurrentFrame();

  virtual void OnTextInputStateChanged();
  virtual void OnFocusedNodeChanged();
  virtual void OnSelectionBoundsChanged();

  virtual void OnSecurityStatusChanged(const SecurityStatus& old);
  virtual bool OnCertificateError(
      bool is_main_frame,
      CertError cert_error,
      const scoped_refptr<net::X509Certificate>& cert,
      const GURL& request_url,
      content::ResourceType resource_type,
      bool strict_enforcement,
      scoped_ptr<SimplePermissionRequest> request);
  virtual void OnContentBlocked();

  virtual void DidGetRedirectForResourceRequest(
      content::RenderViewHost* render_view_host,
      const content::ResourceRedirectDetails& details) FINAL;

  scoped_ptr<content::WebContentsImpl> web_contents_;
  WebViewContentsHelper* web_contents_helper_;

  scoped_ptr<Compositor> compositor_;

  scoped_refptr<CompositorFrameHandle> current_compositor_frame_;
  std::vector<scoped_refptr<CompositorFrameHandle> > previous_compositor_frames_;
  std::queue<uint32> received_surface_ids_;

  scoped_ptr<GestureProvider> gesture_provider_;
  bool in_swap_;

  GURL initial_url_;
  scoped_ptr<content::NavigationController::LoadURLParams> initial_data_;
  WebPreferences* initial_preferences_;

  content::NotificationRegistrar registrar_;
  WebFrame* root_frame_;
  bool is_fullscreen_;
  base::WeakPtr<WebPopupMenu> active_popup_menu_;
  base::WeakPtr<FilePicker> active_file_picker_;

  PermissionRequestManager permission_request_manager_;

  ContentType blocked_content_;

  cc::CompositorFrameMetadata compositor_frame_metadata_;

  SecurityStatus security_status_;

  DISALLOW_COPY_AND_ASSIGN(WebView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_VIEW_H_
