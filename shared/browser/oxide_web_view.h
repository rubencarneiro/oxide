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

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"

#include "shared/browser/oxide_browser_process_handle.h"

class GURL;

namespace gfx {
class Size;
}

namespace content {

struct OpenURLParams;
class WebContents;

} // namespace content

namespace oxide {

class BrowserContext;
class WebContentsViewDelegate;
class WebFrame;

// This is the main webview class. Implementations should subclass
// this. Note that this class will hold the main browser process
// components alive
class WebView : public content::WebContentsDelegate,
                public content::WebContentsObserver {
 public:
  virtual ~WebView();

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

  WebFrame* GetRootFrame() const;

  void DidCommitProvisionalLoadForFrame(
      int64 frame_id,
      bool is_main_frame,
      const GURL& url,
      content::PageTransition transition_type,
      content::RenderViewHost* render_view_host) FINAL;

  void FrameAttached(content::RenderViewHost* render_view_host,
                     int64 parent_frame_id,
                     int64 frame_id) FINAL;
  void FrameDetached(content::RenderViewHost* render_view_host,
                     int64 frame_id) FINAL;

 protected:
  WebView();
  bool Init(BrowserContext* context,
            WebContentsViewDelegate* delegate,
            bool incognito,
            const gfx::Size& initial_size);

  content::WebContents* web_contents() const {
    return web_contents_.get();
  }

  void DestroyWebContents();

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
  WebFrame* FindFrameByID(int64 frame_id);

  virtual void OnURLChanged();
  virtual void OnTitleChanged();
  virtual void OnLoadingChanged();
  virtual void OnCommandsUpdated();
  virtual void OnRootFrameChanged();

  virtual WebFrame* AllocWebFrame(int64 frame_id);

  // Don't mess with the ordering of this unless you know what you
  // are doing. The WebContents needs to disappear first, and the
  // BrowserProcessHandle must outive everything
  BrowserProcessHandle process_handle_;
  scoped_ptr<content::WebContents> web_contents_;
  scoped_ptr<WebFrame> root_frame_;
  NotificationObserver notification_observer_;
  content::NotificationRegistrar registrar_;

  DISALLOW_COPY_AND_ASSIGN(WebView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_VIEW_H_
