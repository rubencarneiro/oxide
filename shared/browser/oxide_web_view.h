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
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "ui/gfx/rect.h"

#include "shared/browser/oxide_browser_process_handle.h"
#include "shared/browser/oxide_message_dispatcher_browser.h"
#include "shared/browser/oxide_message_target.h"

class GURL;

namespace gfx {
class Size;
}

namespace content {

class NavigationController;
struct OpenURLParams;
class RenderWidgetHost;
class RenderWidgetHostView;
class WebContents;

} // namespace content

namespace oxide {

class BrowserContext;
class WebFrame;
class WebPopupMenu;

// This is the main webview class. Implementations should subclass
// this. Note that this class will hold the main browser process
// components alive
class WebView : public MessageTarget,
                public content::WebContentsDelegate,
                public content::WebContentsObserver {
 public:
  virtual ~WebView();

  bool Init(BrowserContext* context,
            bool incognito,
            const gfx::Size& initial_size);
  void Shutdown();

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

  content::NavigationController& GetNavigationController() const;

  WebFrame* GetRootFrame() const;
  WebFrame* FindFrameWithID(int64 frame_id) const;

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
      bool is_main_frame,
      const GURL& url,
      content::PageTransition transition_type,
      content::RenderViewHost* render_view_host) FINAL;

  void DidFailProvisionalLoad(
      int64 frame_id,
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

  virtual size_t GetMessageHandlerCount() const OVERRIDE;
  virtual MessageHandler* GetMessageHandlerAt(size_t index) const OVERRIDE;

  content::WebContents* web_contents() const {
    return web_contents_.get();
  }

  virtual void RootFrameCreated(WebFrame* root);

  virtual content::RenderWidgetHostView* CreateViewForWidget(
      content::RenderWidgetHost* render_widget_host) = 0;

  virtual gfx::Rect GetContainerBounds() = 0;

  virtual WebPopupMenu* CreatePopupMenu(content::RenderViewHost* rvh);

 protected:
  WebView();

 private:

  // Both WebContentsObserver and NotificationObserver have an Observe method
  class NotificationObserver FINAL : public content::NotificationObserver {
   public:
    NotificationObserver(WebView* web_view);

    void Observe(int type,
                 const content::NotificationSource& source,
                 const content::NotificationDetails& details) FINAL;

   private:
    WebView* web_view_;

    DISALLOW_IMPLICIT_CONSTRUCTORS(NotificationObserver);
  };

  void NavigationStateChanged(const content::WebContents* source,
                              unsigned changed_flags) FINAL;
  void NotifyRenderViewHostSwappedIn();

  void DispatchLoadFailed(const GURL& validated_url,
                          int error_code,
                          const base::string16& error_description);

  virtual void OnURLChanged();
  virtual void OnTitleChanged();
  virtual void OnCommandsUpdated();

  virtual void OnRootFrameChanged();

  virtual void OnLoadStarted(const GURL& validated_url,
                             bool is_error_frame);
  virtual void OnLoadStopped(const GURL& validated_url);
  virtual void OnLoadFailed(const GURL& validated_url,
                            int error_code,
                            const std::string& error_description);
  virtual void OnLoadSucceeded(const GURL& validated_url);

  // Don't mess with the ordering of this unless you know what you
  // are doing. The WebContents needs to disappear first, and the
  // BrowserProcessHandle must outive everything
  BrowserProcessHandle process_handle_;
  scoped_ptr<content::WebContents> web_contents_;
  WebFrame* root_frame_;
  NotificationObserver notification_observer_;
  content::NotificationRegistrar registrar_;

  DISALLOW_COPY_AND_ASSIGN(WebView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_VIEW_H_
