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

#ifndef _OXIDE_QT_CORE_BROWSER_WEB_VIEW_H_
#define _OXIDE_QT_CORE_BROWSER_WEB_VIEW_H_

#include <QKeyEvent>
#include <QList>
#include <QPointer>
#include <QSharedPointer>
#include <QtGlobal>

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"

#include "qt/core/browser/input/oxide_qt_input_method_context_client.h"
#include "qt/core/browser/oxide_qt_event_utils.h"
#include "qt/core/glue/oxide_qt_web_view_proxy.h"
#include "shared/browser/oxide_javascript_dialog_manager.h"
#include "shared/browser/oxide_web_view_client.h"
#include "shared/browser/oxide_web_frame_tree_observer.h"
#include "shared/browser/permissions/oxide_permission_request_dispatcher_client.h"
#include "shared/browser/ssl/oxide_certificate_error_dispatcher_client.h"

QT_BEGIN_NAMESPACE
class QFocusEvent;
class QInputMethodEvent;
class QKeyEvent;
class QMouseEvent;
class QWheelEvent;
QT_END_NAMESPACE

class OxideQFindController;
class OxideQSecurityStatus;

namespace oxide {

class WebView;

namespace qt {

class CompositorFrameHandle;
class InputMethodContext;
class WebContext;
class WebViewProxyClient;

class WebView : public InputMethodContextClient,
                public oxide::WebViewClient,
                public oxide::PermissionRequestDispatcherClient,
                public oxide::WebFrameTreeObserver,
                public oxide::CertificateErrorDispatcherClient,
                public WebViewProxy {
 public:
  WebView(WebViewProxyClient* client,
          OxideQFindController* find_controller,
          OxideQSecurityStatus* security_status,
          WebContext* context,
          bool incognito,
          const QByteArray& restore_state,
          RestoreType restore_type);
  static WebView* CreateFromNewViewRequest(
      WebViewProxyClient* client,
      OxideQFindController* find_controller,
      OxideQSecurityStatus* security_status,
      OxideQNewViewRequest* new_view_request);
  ~WebView();

  static WebView* FromProxyHandle(WebViewProxyHandle* handle);
  static WebView* FromView(oxide::WebView* view);

  WebContext* GetContext() const;

  const oxide::SecurityStatus& GetSecurityStatus() const;

  float GetDeviceScaleFactor() const;

 private:
  WebView(WebViewProxyClient* client,
          OxideQSecurityStatus* security_status);

  void CommonInit(OxideQFindController* find_controller);

  void EnsurePreferences();

  // InputMethodContextClient implementation
  void SetInputMethodEnabled(bool enabled);

  // oxide::WebViewClient implementation
  blink::WebScreenInfo GetScreenInfo() const override;
  gfx::Rect GetViewBoundsPix() const override;
  bool IsVisible() const override;
  bool HasFocus() const override;
  oxide::JavaScriptDialog* CreateJavaScriptDialog(
      content::JavaScriptMessageType javascript_message_type) override;
  oxide::JavaScriptDialog* CreateBeforeUnloadDialog() override;
  bool CanCreateWindows() const override;
  void CrashedStatusChanged() override;
  void URLChanged() override;
  void TitleChanged() override;
  void FaviconChanged() override;
  void CommandsUpdated() override;
  void LoadingChanged() override;
  void LoadProgressChanged(double progress) override;
  void LoadStarted(const GURL& validated_url) override;
  void LoadRedirected(const GURL& url,
                      const GURL& original_url,
                      int http_status_code) override;
  void LoadCommitted(const GURL& url,
                     bool is_error_page,
                     int http_status_code) override;
  void LoadStopped(const GURL& validated_url) override;
  void LoadFailed(const GURL& validated_url,
                  int error_code,
                  const std::string& error_description,
                  int http_status_code) override;
  void LoadSucceeded(const GURL& validated_url,
                     int http_status_code) override;
  void NavigationEntryCommitted() override;
  void NavigationListPruned(bool from_front, int count) override;
  void NavigationEntryChanged(int index) override;
  bool AddMessageToConsole(int32_t level,
                           const base::string16& message,
                           int32_t line_no,
                           const base::string16& source_id) override;
  void ToggleFullscreenMode(bool enter) override;
  void WebPreferencesDestroyed() override;
  void UnhandledKeyboardEvent(
      const content::NativeWebKeyboardEvent& event) override;
  void FrameMetadataUpdated(const cc::CompositorFrameMetadata& old) override;
  void DownloadRequested(const GURL& url,
      const std::string& mime_type,
      const bool should_prompt,
      const base::string16& suggested_filename,
      const std::string& cookies,
      const std::string& referrer,
      const std::string& user_agent) override;
  void HttpAuthenticationRequested(
      ResourceDispatcherHostLoginDelegate* login_delegate) override;

  bool ShouldHandleNavigation(const GURL& url,
                              WindowOpenDisposition disposition,
                              bool user_gesture) override;
  oxide::WebContextMenu* CreateContextMenu(
      content::RenderFrameHost* rfh,
      const content::ContextMenuParams& params) override;
  oxide::WebPopupMenu* CreatePopupMenu(content::RenderFrameHost* rfh) override;
  oxide::WebView* CreateNewWebView(
      const gfx::Rect& initial_pos,
      WindowOpenDisposition disposition,
      scoped_ptr<content::WebContents> contents) override;
  oxide::FilePicker* CreateFilePicker(content::RenderViewHost* rvh) override;
  oxide::TouchHandleDrawableDelegate* CreateTouchHandleDrawableDelegate() const override;
  void TouchSelectionChanged(
      bool active,
      gfx::RectF bounds,
      int edit_flags,
      const base::string16& selected_text) const override;
  void SwapCompositorFrame() override;
  void EvictCurrentFrame() override;
  oxide::InputMethodContext* GetInputMethodContext() const override;
  void UpdateCursor(const content::WebCursor& cursor) override;
  void SecurityStatusChanged(const oxide::SecurityStatus& old) override;
  void ContentBlocked() override;
  void PrepareToCloseResponseReceived(bool proceed) override;
  void CloseRequested() override;

  // oxide::ScriptMessageTarget implementation
  size_t GetScriptMessageHandlerCount() const override;
  const oxide::ScriptMessageHandler* GetScriptMessageHandlerAt(
      size_t index) const override;

  // oxide::PermissionRequestDispatcherClient implementation
  void RequestGeolocationPermission(
      scoped_ptr<oxide::SimplePermissionRequest> request) override;
  void RequestNotificationPermission(
      scoped_ptr<oxide::SimplePermissionRequest> request) override;
  void RequestMediaAccessPermission(
      scoped_ptr<oxide::MediaAccessPermissionRequest> request) override;

  // oxide::WebFrameTreeObserver implementation
  void FrameCreated(oxide::WebFrame* frame) override;
  void FrameDeleted(oxide::WebFrame* frame) override;
  void LoadCommittedInFrame(oxide::WebFrame* frame) override;

  // oxide::CertificateErrorDispatcherClient implementation
  void OnCertificateError(scoped_ptr<oxide::CertificateError> error) override;

  // WebViewProxy implementation
  QUrl url() const override;
  void setUrl(const QUrl& url) override;

  QString title() const override;

  QUrl favIconUrl() const override;

  bool canGoBack() const override;
  bool canGoForward() const override;

  bool incognito() const override;

  bool loading() const override;

  bool fullscreen() const override;
  void setFullscreen(bool fullscreen) override;

  WebFrameProxyHandle* rootFrame() const override;

  WebContextProxyHandle* context() const override;

  void wasResized() override;
  void screenUpdated() override;
  void visibilityChanged() override;

  void handleFocusEvent(QFocusEvent* event) override;
  void handleHoverEvent(QHoverEvent* event,
                        const QPoint& window_pos,
                        const QPoint& global_pos) override;
  void handleInputMethodEvent(QInputMethodEvent* event) override;
  void handleKeyEvent(QKeyEvent* event) override;
  void handleMouseEvent(QMouseEvent* event) override;
  void handleTouchEvent(QTouchEvent* event) override;
  void handleWheelEvent(QWheelEvent* event,
                        const QPoint& window_pos) override;

  QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

  void goBack() override;
  void goForward() override;
  void stop() override;
  void reload() override;

  void loadHtml(const QString& html, const QUrl& base_url) override;

  QList<ScriptMessageHandlerProxyHandle*>& messageHandlers() override;

  int getNavigationEntryCount() const override;
  int getNavigationCurrentEntryIndex() const override;
  void setNavigationCurrentEntryIndex(int index) override;
  int getNavigationEntryUniqueID(int index) const override;
  QUrl getNavigationEntryUrl(int index) const override;
  QString getNavigationEntryTitle(int index) const override;
  QDateTime getNavigationEntryTimestamp(int index) const override;

  QByteArray currentState() const override;

  OxideQWebPreferences* preferences() override;
  void setPreferences(OxideQWebPreferences* prefs) override;

  void updateWebPreferences() override;

  QPoint compositorFrameScrollOffsetPix() override;
  QSize compositorFrameContentSizePix() override;
  QSize compositorFrameViewportSizePix() override;

  QSharedPointer<CompositorFrameHandle> compositorFrameHandle() override;
  void didCommitCompositorFrame() override;

  void setCanTemporarilyDisplayInsecureContent(bool allow) override;
  void setCanTemporarilyRunInsecureContent(bool allow) override;;

  ContentTypeFlags blockedContent() const override;

  void prepareToClose() override;

  int locationBarHeight() override;
  void setLocationBarHeight(int height) override;
  int locationBarOffsetPix() override;
  int locationBarContentOffsetPix() override;
  LocationBarMode locationBarMode() const override;
  void setLocationBarMode(LocationBarMode mode) override;
  bool locationBarAnimated() const override;
  void setLocationBarAnimated(bool animated) override;
  void locationBarShow(bool animate) override;
  void locationBarHide(bool animate) override;

  WebProcessStatus webProcessStatus() const override;

  void executeEditingCommand(EditingCommands command) const override;

  void teardownFrameTree() override;

  // This must outlive |view_|
  scoped_ptr<InputMethodContext> input_method_context_;

  scoped_ptr<oxide::WebView> view_;

  WebViewProxyClient* client_;

  QPointer<OxideQSecurityStatus> security_status_;
  QList<ScriptMessageHandlerProxyHandle*> message_handlers_;

  UITouchEventFactory touch_event_factory_;

  QSharedPointer<CompositorFrameHandle> compositor_frame_;

  bool frame_tree_torn_down_;

  DISALLOW_COPY_AND_ASSIGN(WebView);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_WEB_VIEW_H_
