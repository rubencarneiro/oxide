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
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "ui/gfx/rect.h"

#include "shared/browser/oxide_message_target.h"
#include "shared/common/oxide_message_enums.h"

struct OxideMsg_SendMessage_Params;

class GURL;

namespace gfx {
class Size;
}

namespace content {

class NotificationRegistrar;
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
                public content::NotificationObserver,
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

  int GetNavigationEntryCount() const;
  int GetNavigationCurrentEntryIndex() const;
  void SetNavigationCurrentEntryIndex(int index);
  int GetNavigationEntryUniqueID(int index) const;
  const GURL& GetNavigationEntryUrl(int index) const;
  std::string GetNavigationEntryTitle(int index) const;
  base::Time GetNavigationEntryTimestamp(int index) const;

  WebFrame* GetRootFrame() const;
  WebFrame* FindFrameWithID(int64 frame_id) const;

  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) FINAL;

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
                     int64 frame_id) FINAL;

  void TitleWasSet(content::NavigationEntry* entry, bool explicit_set) FINAL;

  bool OnMessageReceived(const IPC::Message& message) FINAL;

  virtual size_t GetMessageHandlerCount() const OVERRIDE;
  virtual MessageHandler* GetMessageHandlerAt(size_t index) const OVERRIDE;

  content::WebContents* web_contents() const {
    return web_contents_.get();
  }

  virtual content::RenderWidgetHostView* CreateViewForWidget(
      content::RenderWidgetHost* render_widget_host) = 0;

  virtual gfx::Rect GetContainerBounds() = 0;

  virtual WebPopupMenu* CreatePopupMenu(content::RenderViewHost* rvh);

  virtual void FrameAdded(WebFrame* frame);
  virtual void FrameRemoved(WebFrame* frame);

 protected:
  WebView();

 private:
  void NavigationStateChanged(const content::WebContents* source,
                              unsigned changed_flags) FINAL;

  void LoadProgressChanged(content::WebContents* source, double progress) FINAL;

  void DispatchLoadFailed(const GURL& validated_url,
                          int error_code,
                          const base::string16& error_description);

  void SendErrorForV8Message(long long frame_id,
                             const std::string& world_id,
                             int serial,
                             OxideMsg_SendMessage_Error::Value error_code,
                             const std::string& error_desc);
  bool TryDispatchV8MessageToTarget(MessageTarget* target,
                                    WebFrame* source_frame,
                                    const std::string& world_id,
                                    int serial,
                                    const std::string& msg_id,
                                    const std::string& args);
  void DispatchV8Message(const OxideMsg_SendMessage_Params& params);

  void OnFrameCreated(int64 parent_frame_id, int64 frame_id);

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

  virtual WebFrame* CreateWebFrame() = 0;

  scoped_ptr<content::WebContents> web_contents_;
  WebFrame* root_frame_;

  scoped_ptr<content::NotificationRegistrar> registrar_;

  DISALLOW_COPY_AND_ASSIGN(WebView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_VIEW_H_
