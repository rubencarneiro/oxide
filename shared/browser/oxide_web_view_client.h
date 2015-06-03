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
class WebCursor;
}

namespace oxide {

class CertificateError;
class FilePicker;
class JavaScriptDialog;
class LoginPromptDelegate;
class MediaAccessPermissionRequest;
class SecurityStatus;
class SimplePermissionRequest;
class WebContextMenu;
class WebFrame;
class WebPopupMenu;
class WebView;

// A class for customizing the behaviour of WebView
// TODO(chrisccoulson): Redesign ScriptMessageTarget and stop inheriting it
class WebViewClient : public ScriptMessageTarget {
 public:
  virtual ~WebViewClient();

  virtual void Initialized();

  virtual blink::WebScreenInfo GetScreenInfo() const = 0;

  virtual gfx::Rect GetViewBoundsPix() const = 0;

  virtual bool IsVisible() const = 0;

  virtual bool HasFocus() const = 0;

  // XXX(chrisccoulson): This is global state, so it doesn't belong here
  virtual bool IsInputPanelVisible() const;

  // TODO(chrisccoulson): Make a delegate for JavaScriptDialogManager and move there
  virtual JavaScriptDialog* CreateJavaScriptDialog(
      content::JavaScriptMessageType javascript_message_type);

  // TODO(chrisccoulson): Make a delegate for JavaScriptDialogManager and move there
  virtual JavaScriptDialog* CreateBeforeUnloadDialog();

  virtual bool CanCreateWindows() const;

  virtual void CrashedStatusChanged();

  virtual void URLChanged();

  virtual void TitleChanged();

  // TODO(chrisccoulson): Track |icon| as a property in WebView
  virtual void IconChanged(const GURL& icon);

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

  // TODO(chrisccoulson): Make a delegate for dispatching permission requests
  //    and move there
  virtual void RequestGeolocationPermission(
      scoped_ptr<SimplePermissionRequest> request);

  // TODO(chrisccoulson): Make a delegate for dispatching permission requests
  //    and move there
  virtual void RequestMediaAccessPermission(
      scoped_ptr<MediaAccessPermissionRequest> request);

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

  virtual WebFrame* CreateWebFrame(
      content::RenderFrameHost* render_frame_host);

  virtual WebContextMenu* CreateContextMenu(
      content::RenderFrameHost* rfh,
      const content::ContextMenuParams& params);

  virtual WebPopupMenu* CreatePopupMenu(content::RenderFrameHost* rfh);

  virtual WebView* CreateNewWebView(const gfx::Rect& initial_pos,
                                    WindowOpenDisposition disposition);

  virtual FilePicker* CreateFilePicker(content::RenderViewHost* rvh);

  virtual void SwapCompositorFrame() = 0;

  virtual void EvictCurrentFrame();

  // XXX(chrisccoulson): Rethink all of these IME related bits:
  //    - Move some logic down from qt/ to shared/
  //    - The implementations of some of these only touch process-global
  //      stuff - should we have an InputMethod singleton in shared/
  //      rather than dumping it all in WebView?
  virtual void TextInputStateChanged();

  virtual void FocusedNodeChanged();

  virtual void SelectionBoundsChanged();

  virtual void ImeCancelComposition();

  virtual void SelectionChanged();

  virtual void UpdateCursor(const content::WebCursor& cursor);

  // TODO(chrisccoulson): Get rid of |old| and add |changed_flags|
  virtual void SecurityStatusChanged(const SecurityStatus& old);

  virtual void OnCertificateError(scoped_ptr<CertificateError> error);

  // TODO(chrisccoulson): Rename to BlockedContentChanged or something
  // TODO(chrisccoulson): Move content tracking to a separate class with its
  //    own delegate, as this is going to be expanded with content settings
  //    work
  virtual void ContentBlocked();

  virtual void PrepareToCloseResponseReceived(bool proceed);

  virtual void CloseRequested();

  virtual void FindInPageCurrentChanged();
  virtual void FindInPageCountChanged();

  virtual void BasicAuthenticationRequested(
      LoginPromptDelegate* login_delegate);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_VIEW_CLIENT_H_
