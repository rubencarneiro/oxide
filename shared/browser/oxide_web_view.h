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
#include "cc/output/compositor_frame_metadata.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/javascript_message_type.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"
#include "ui/gfx/rect.h"

#include "shared/browser/compositor/oxide_compositor_client.h"
#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_permission_request.h"
#include "shared/browser/oxide_script_message_target.h"
#include "shared/browser/oxide_web_preferences_observer.h"
#include "shared/browser/oxide_web_view_contents_helper_delegate.h"
#include "shared/common/oxide_message_enums.h"

class GURL;

namespace gfx {
class Size;
}

namespace content {

class FrameTree;
class FrameTreeNode;
struct MenuItem;
class NotificationRegistrar;
struct OpenURLParams;
class WebContents;
class WebContentsImpl;

} // namespace content

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

  void ShowPopupMenu(const gfx::Rect& bounds,
                     int selected_item,
                     const std::vector<content::MenuItem>& items,
                     bool allow_multiple_selection);
  void HidePopupMenu();

  void RequestGeolocationPermission(
      int id,
      const GURL& origin,
      const base::Callback<void(bool)>& callback);
  void CancelGeolocationPermissionRequest(int id);

  void UpdateWebPreferences();

  Compositor* compositor() const { return compositor_.get(); }

  CompositorFrameHandle* GetCompositorFrameHandle();
  void DidCommitCompositorFrame();

  void EvictCurrentFrame();
  void GotNewCompositorFrameMetadata(
      const cc::CompositorFrameMetadata& metadata);

  virtual blink::WebScreenInfo GetScreenInfo() const = 0;
  virtual gfx::Rect GetContainerBoundsPix() const = 0;
  virtual bool IsVisible() const = 0;
  virtual bool HasFocus() const = 0;

  virtual JavaScriptDialog* CreateJavaScriptDialog(
      content::JavaScriptMessageType javascript_message_type,
      bool* did_suppress_message);
  virtual JavaScriptDialog* CreateBeforeUnloadDialog();

  virtual FilePicker* CreateFilePicker(content::RenderViewHost* rvh);

  virtual void FrameAdded(WebFrame* frame);
  virtual void FrameRemoved(WebFrame* frame);

  virtual bool CanCreateWindows() const;

 protected:
  WebView();

  float GetDeviceScaleFactor() const;
  float GetPageScaleFactor() const;
  virtual void PageScaleFactorChanged();

  const gfx::Vector2dF& GetRootScrollOffset() const;
  virtual void RootScrollOffsetXChanged();
  virtual void RootScrollOffsetYChanged();

  const gfx::SizeF& GetRootLayerSize() const;
  virtual void RootLayerWidthChanged();
  virtual void RootLayerHeightChanged();

  const gfx::SizeF& GetViewportSize() const;
  virtual void ViewportWidthChanged();
  virtual void ViewportHeightChanged();

 private:
  RenderWidgetHostView* GetRenderWidgetHostView();
  void DispatchLoadFailed(const GURL& validated_url,
                          int error_code,
                          const base::string16& error_description);

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

  // content::NotificationObserver implementation
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) FINAL;

  // WebViewContentsHelperDelegate implementation
  content::WebContents* OpenURL(const content::OpenURLParams& params) FINAL;
  void NavigationStateChanged(unsigned flags) FINAL;
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
  void HandleKeyboardEvent(const content::NativeWebKeyboardEvent& event) FINAL;

  // content::WebContentsObserver implementation
  void RenderProcessGone(base::TerminationStatus status) FINAL;
  void RenderViewHostChanged(content::RenderViewHost* old_host,
                             content::RenderViewHost* new_host) FINAL;

  void DidStartProvisionalLoadForFrame(
      int64 frame_id,
      int64 parent_frame_id,
      bool is_main_frame,
      const GURL& validated_url,
      bool is_error_page,
      bool is_iframe_srcdoc,
      content::RenderViewHost* render_view_host) FINAL;

  void DidCommitProvisionalLoadForFrame(
      int64 frame_id,
      const base::string16& frame_unique_name,
      bool is_main_frame,
      const GURL& url,
      content::PageTransition transition_type,
      content::RenderViewHost* render_view_host) FINAL;

  void DidFailProvisionalLoad(
      int64 frame_id,
      const base::string16& frame_unique_name,
      bool is_main_frame,
      const GURL& validated_url,
      int error_code,
      const base::string16& error_description,
      content::RenderViewHost* render_view_host) FINAL;

  void DidFinishLoad(int64 frame_id,
                     const GURL& validated_url,
                     bool is_main_frame,
                     content::RenderViewHost* render_view_host);
  void DidFailLoad(int64 frame_id,
                   const GURL& validated_url,
                   bool is_main_frame,
                   int error_code,
                   const base::string16& error_description,
                   content::RenderViewHost* render_view_host) FINAL;

  void NavigationEntryCommitted(
      const content::LoadCommittedDetails& load_details) FINAL;

  void FrameDetached(content::RenderViewHost* rvh,
                     int64 frame_routing_id) FINAL;
  void FrameAttached(content::RenderViewHost* rvh,
                     int64 parent_frame_routing_id,
                     int64 frame_routing_id) FINAL;

  void TitleWasSet(content::NavigationEntry* entry, bool explicit_set) FINAL;

  void DidUpdateFaviconURL(
      const std::vector<content::FaviconURL>& candidates) FINAL;

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
      scoped_ptr<GeolocationPermissionRequest> request);

  virtual void OnUnhandledKeyboardEvent(
      const content::NativeWebKeyboardEvent& event);

  virtual bool ShouldHandleNavigation(const GURL& url,
                                      WindowOpenDisposition disposition,
                                      bool user_gesture);

  virtual WebFrame* CreateWebFrame(content::FrameTreeNode* node) = 0;
  virtual WebPopupMenu* CreatePopupMenu(content::RenderViewHost* rvh);

  virtual WebView* CreateNewWebView(const gfx::Rect& initial_pos,
                                    WindowOpenDisposition disposition);

  virtual void OnSwapCompositorFrame() = 0;
  virtual void OnEvictCurrentFrame();

  scoped_ptr<content::WebContentsImpl> web_contents_;
  WebViewContentsHelper* web_contents_helper_;

  scoped_ptr<Compositor> compositor_;

  scoped_refptr<CompositorFrameHandle> current_compositor_frame_;
  std::vector<scoped_refptr<CompositorFrameHandle> > previous_compositor_frames_;
  std::queue<uint32> received_surface_ids_;

  GURL initial_url_;
  WebPreferences* initial_preferences_;

  content::NotificationRegistrar registrar_;
  WebFrame* root_frame_;
  bool is_fullscreen_;
  base::WeakPtr<WebPopupMenu> active_popup_menu_;
  base::WeakPtr<FilePicker> active_file_picker_;

  PermissionRequestManager geolocation_permission_requests_;

  cc::CompositorFrameMetadata frame_metadata_;

  DISALLOW_COPY_AND_ASSIGN(WebView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_VIEW_H_
