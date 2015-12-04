// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_WEB_VIEW_CLIENT_H_
#define _OXIDE_SHARED_BROWSER_WEB_VIEW_CLIENT_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "base/strings/string16.h"
#include "content/public/common/javascript_message_type.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"
#include "ui/base/window_open_disposition.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/rect_f.h"

#include "shared/browser/oxide_script_message_target.h"

class GURL;

namespace cc {
class CompositorFrameMetadata;
}

namespace content {
struct ContextMenuParams;
class NativeWebKeyboardEvent;
class RenderFrameHost;
class RenderViewHost;
class WebContents;
class WebCursor;
}

namespace oxide {

class CertificateError;
class FilePicker;
class InputMethodContext;
class JavaScriptDialog;
class ResourceDispatcherHostLoginDelegate;
class SecurityStatus;
class TouchHandleDrawableDelegate;
class WebContextMenu;
class WebPopupMenu;
class WebView;

// A class for customizing the behaviour of WebView
// TODO(chrisccoulson): Redesign ScriptMessageTarget and stop inheriting it
class WebViewClient : public ScriptMessageTarget {
 public:
  virtual ~WebViewClient();

  virtual blink::WebScreenInfo GetScreenInfo() const = 0;

  virtual gfx::Rect GetViewBoundsPix() const = 0;

  virtual bool IsVisible() const = 0;

  virtual bool HasFocus() const = 0;

  // TODO(chrisccoulson): Make a delegate for JavaScriptDialogManager and move there
  virtual JavaScriptDialog* CreateJavaScriptDialog(
      content::JavaScriptMessageType javascript_message_type);

  // TODO(chrisccoulson): Make a delegate for JavaScriptDialogManager and move there
  virtual JavaScriptDialog* CreateBeforeUnloadDialog();

  virtual bool CanCreateWindows() const;

  virtual void CrashedStatusChanged();

  virtual void URLChanged();

  virtual void TitleChanged();

  virtual void FaviconChanged();

  virtual void CommandsUpdated();

  virtual void LoadingChanged();

  virtual void LoadProgressChanged(double progress);

  virtual void LoadStarted(const GURL& validated_url);

  virtual void LoadRedirected(const GURL& url,
                              const GURL& original_url,
                              int http_status_code);

  virtual void LoadCommitted(const GURL& url,
                             bool is_error_page,
                             int http_status_code);

  virtual void LoadStopped(const GURL& validated_url);

  virtual void LoadFailed(const GURL& validated_url,
                          int error_code,
                          const std::string& error_description,
                          int http_status_code);

  virtual void LoadSucceeded(const GURL& validated_url,
                             int http_status_code);

  virtual void NavigationEntryCommitted();

  virtual void NavigationListPruned(bool from_front, int count);

  virtual void NavigationEntryChanged(int index);

  virtual bool AddMessageToConsole(int32_t level,
                                   const base::string16& message,
                                   int32_t line_no,
                                   const base::string16& source_id);

  virtual void ToggleFullscreenMode(bool enter);

  // TODO(chrisccoulson): Make WebPreferences ref-counted and get rid of this
  virtual void WebPreferencesDestroyed();

  virtual void UnhandledKeyboardEvent(
      const content::NativeWebKeyboardEvent& event);

  // TODO(chrisccoulson): Merge with SwapCompositorFrame
  // TODO(chrisccoulson): Get rid of |old| and replace with |changed_flags|
  virtual void FrameMetadataUpdated(const cc::CompositorFrameMetadata& old);

  // XXX(chrisccoulson): WebView currently just proxies straight to this -
  //    should this get its own interface?
  virtual void DownloadRequested(const GURL& url,
                                 const std::string& mime_type,
                                 const bool should_prompt,
                                 const base::string16& suggested_filename,
                                 const std::string& cookies,
                                 const std::string& referrer,
                                 const std::string& user_agent);

  virtual bool ShouldHandleNavigation(const GURL& url,
                                      WindowOpenDisposition disposition,
                                      bool user_gesture);

  virtual WebContextMenu* CreateContextMenu(
      content::RenderFrameHost* rfh,
      const content::ContextMenuParams& params);

  virtual WebPopupMenu* CreatePopupMenu(content::RenderFrameHost* rfh);

  virtual WebView* CreateNewWebView(const gfx::Rect& initial_pos,
                                    WindowOpenDisposition disposition,
                                    scoped_ptr<content::WebContents> contents);

  virtual FilePicker* CreateFilePicker(content::RenderViewHost* rvh);

  virtual TouchHandleDrawableDelegate* CreateTouchHandleDrawableDelegate() const;
  virtual void TouchSelectionChanged(bool active,
                                     gfx::RectF bounds,
                                     int edit_flags) const;

  virtual void SwapCompositorFrame() = 0;

  virtual void EvictCurrentFrame();

  virtual InputMethodContext* GetInputMethodContext() const;

  virtual void UpdateCursor(const content::WebCursor& cursor);

  // TODO(chrisccoulson): Get rid of |old| and add |changed_flags|
  virtual void SecurityStatusChanged(const SecurityStatus& old);

  // TODO(chrisccoulson): Rename to BlockedContentChanged or something
  // TODO(chrisccoulson): Move content tracking to a separate class with its
  //    own delegate, as this is going to be expanded with content settings
  //    work
  virtual void ContentBlocked();

  virtual void PrepareToCloseResponseReceived(bool proceed);

  virtual void CloseRequested();

  virtual void TargetURLChanged();

  virtual void HttpAuthenticationRequested(
      ResourceDispatcherHostLoginDelegate* login_delegate);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_VIEW_CLIENT_H_
