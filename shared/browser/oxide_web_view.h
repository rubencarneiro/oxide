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

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/javascript_dialog_manager.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/javascript_message_type.h"
#include "ui/gfx/rect.h"

#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_browser_context_observer.h"
#include "shared/browser/oxide_script_message_target.h"
#include "shared/browser/oxide_web_preferences_observer.h"
#include "shared/common/oxide_message_enums.h"

class GURL;

namespace gfx {
class Size;
}

namespace content {

class FrameTreeNode;
struct MenuItem;
class NotificationRegistrar;
struct OpenURLParams;
class RenderWidgetHost;
class RenderWidgetHostView;
class WebContents;
class WebContentsImpl;

} // namespace content

namespace oxide {

class BrowserContext;
class JavaScriptDialog;
class WebFrame;
class WebPopupMenu;
class WebPreferences;

// This is the main webview class. Implementations should subclass
// this. Note that this class will hold the main browser process
// components alive
class WebView : public ScriptMessageTarget,
                public BrowserContextObserver,
                public WebPreferencesObserver,
                public content::NotificationObserver,
                public content::WebContentsDelegate,
                public content::WebContentsObserver {
 public:
  virtual ~WebView();

  bool Init(BrowserContext* context,
            bool incognito,
            const gfx::Size& initial_size);

  static WebView* FromWebContents(content::WebContents* web_contents);
  static WebView* FromRenderViewHost(content::RenderViewHost* rvh);

  const GURL& GetURL() const;
  void SetURL(const GURL& url);

  std::string GetTitle() const;

  bool CanGoBack() const;
  bool CanGoForward() const;

  void GoBack();
  void GoForward();
  void Stop();
  void Reload();

  bool IsIncognito() const;

  bool IsLoading() const;

  void UpdateSize(const gfx::Size& size);
  void Shown();
  void Hidden();

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

  virtual content::RenderWidgetHostView* CreateViewForWidget(
      content::RenderWidgetHost* render_widget_host) = 0;

  virtual gfx::Rect GetContainerBounds() = 0;

  virtual JavaScriptDialog* CreateJavaScriptDialog(
      content::JavaScriptMessageType javascript_message_type,
      bool* did_suppress_message);
  virtual JavaScriptDialog* CreateBeforeUnloadDialog();

  virtual void FrameAdded(WebFrame* frame);
  virtual void FrameRemoved(WebFrame* frame);

  void ShowPopupMenu(const gfx::Rect& bounds,
                     int selected_item,
                     const std::vector<content::MenuItem>& items,
                     bool allow_multiple_selection);
  void HidePopupMenu();

 protected:
  WebView();

 private:
  void DispatchLoadFailed(const GURL& validated_url,
                          int error_code,
                          const base::string16& error_description);

  // ScriptMessageTarget
  virtual size_t GetScriptMessageHandlerCount() const OVERRIDE;
  virtual ScriptMessageHandler* GetScriptMessageHandlerAt(
      size_t index) const OVERRIDE;

  // BrowserContextObserver
  void NotifyUserAgentStringChanged() FINAL;

  // WebPreferencesObserver
  void WebPreferencesDestroyed() FINAL;
  void WebPreferencesValueChanged() FINAL;

  // content::NotificationObserver
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) FINAL;

  // content::WebContentsDelegate
  void NavigationStateChanged(const content::WebContents* source,
                              unsigned changed_flags) FINAL;
  void LoadProgressChanged(content::WebContents* source, double progress) FINAL;
  content::JavaScriptDialogManager* GetJavaScriptDialogManager() FINAL;

  // content::WebContentsObserver
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

  virtual void OnURLChanged();
  virtual void OnTitleChanged();
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

  virtual void OnWebPreferencesChanged();

  virtual WebFrame* CreateWebFrame(content::FrameTreeNode* node) = 0;
  virtual WebPopupMenu* CreatePopupMenu(content::RenderViewHost* rvh);

  ScopedBrowserContext context_;
  scoped_ptr<content::WebContentsImpl> web_contents_;
  content::NotificationRegistrar registrar_;
  scoped_ptr<WebFrame> root_frame_;
  base::WeakPtr<WebPopupMenu> active_popup_menu_;

  DISALLOW_COPY_AND_ASSIGN(WebView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_VIEW_H_
