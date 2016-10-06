// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include <memory>

#include <QKeyEvent>
#include <QList>
#include <QPointer>
#include <QtGlobal>

#include "base/macros.h"
#include "content/public/browser/host_zoom_map.h"

#include "qt/core/glue/oxide_qt_web_view_proxy.h"
#include "shared/browser/oxide_fullscreen_helper_client.h"
#include "shared/browser/oxide_javascript_dialog_manager.h"
#include "shared/browser/oxide_web_view_client.h"
#include "shared/browser/oxide_web_frame_tree_observer.h"
#include "shared/browser/permissions/oxide_permission_request_dispatcher_client.h"
#include "shared/browser/web_contents_client.h"

QT_BEGIN_NAMESPACE
class QFocusEvent;
class QInputMethodEvent;
class QKeyEvent;
class QMouseEvent;
class QWheelEvent;
QT_END_NAMESPACE

class OxideQWebPreferences;

namespace oxide {

class WebView;

namespace qt {

class ContentsView;
class ContentsViewProxy;
class ContentsViewProxyClient;
class WebContext;
class WebViewProxyClient;

class WebView : public oxide::WebViewClient,
                public oxide::WebContentsClient,
                public oxide::PermissionRequestDispatcherClient,
                public oxide::WebFrameTreeObserver,
                public oxide::FullscreenHelperClient,
                public WebViewProxy {
 public:
  WebView(WebViewProxyClient* client,
          ContentsViewProxyClient* view_client,
          QObject* handle,
          WebContext* context,
          bool incognito,
          const QByteArray& restore_state,
          RestoreType restore_type);
  static WebView* CreateFromNewViewRequest(
      WebViewProxyClient* client,
      ContentsViewProxyClient* view_client,
      QObject* handle,
      OxideQNewViewRequest* new_view_request,
      OxideQWebPreferences* initial_prefs);
  ~WebView();

  static WebView* FromView(oxide::WebView* view);

  WebContext* GetContext() const;

 private:
  WebView(WebViewProxyClient* client,
          ContentsViewProxyClient* view_client,
          QObject* handle);

  void CommonInit();

  void OnZoomLevelChanged(const content::HostZoomMap::ZoomLevelChange& change);

  // oxide::WebViewClient implementation
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
  oxide::WebView* CreateNewWebView(
      const gfx::Rect& initial_pos,
      WindowOpenDisposition disposition,
      oxide::WebContentsUniquePtr contents) override;
  oxide::FilePicker* CreateFilePicker(content::RenderFrameHost* rfh) override;
  void ContentBlocked() override;
  void PrepareToCloseResponseReceived(bool proceed) override;
  void CloseRequested() override;
  void TargetURLChanged() override;
  void OnEditingCapabilitiesChanged() override;

  // oxide::ScriptMessageTarget implementation
  size_t GetScriptMessageHandlerCount() const override;
  const oxide::ScriptMessageHandler* GetScriptMessageHandlerAt(
      size_t index) const override;

  // oxide::PermissionRequestDispatcherClient implementation
  void RequestGeolocationPermission(
      std::unique_ptr<oxide::PermissionRequest> request) override;
  void RequestNotificationPermission(
      std::unique_ptr<oxide::PermissionRequest> request) override;
  void RequestMediaAccessPermission(
      std::unique_ptr<oxide::MediaAccessPermissionRequest> request) override;

  // oxide::WebFrameTreeObserver implementation
  void FrameCreated(oxide::WebFrame* frame) override;
  void FrameDeleted(oxide::WebFrame* frame) override;
  void LoadCommittedInFrame(oxide::WebFrame* frame) override;

  // oxide::CertificateErrorDispatcher::Callback
  void OnCertificateError(std::unique_ptr<oxide::CertificateError> error);

  // oxide::FullscreenHelperClient
  void EnterFullscreenMode(const GURL& origin) override;
  void ExitFullscreenMode() override;

  // WebViewProxy implementation
  WebContentsID webContentsID() const override;

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

  QObject* rootFrame() const override;

  QObject* context() const override;

  void goBack() override;
  void goForward() override;
  void stop() override;
  void reload() override;

  void loadHtml(const QString& html, const QUrl& base_url) override;

  QList<QObject*>& messageHandlers() override;

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

  void syncWebPreferences() override;

  QPoint compositorFrameScrollOffset() override;
  QSize compositorFrameContentSize() override;
  QSize compositorFrameViewportSize() override;

  void setCanTemporarilyDisplayInsecureContent(bool allow) override;
  void setCanTemporarilyRunInsecureContent(bool allow) override;;

  ContentTypeFlags blockedContent() const override;

  void prepareToClose() override;

  WebProcessStatus webProcessStatus() const override;

  void executeEditingCommand(EditingCommands command) const override;

  QUrl targetUrl() const override;

  EditCapabilityFlags editFlags() const override;

  qreal zoomFactor() const override;
  void setZoomFactor(qreal factor) override;

  void teardownFrameTree() override;

  void killWebProcess(bool crash) override;

  std::unique_ptr<ContentsView> contents_view_;

  std::unique_ptr<oxide::WebView> web_view_;

  WebViewProxyClient* client_;

  QList<QObject*> message_handlers_;

  bool frame_tree_torn_down_;

  std::unique_ptr<content::HostZoomMap::Subscription> track_zoom_subscription_;

  DISALLOW_COPY_AND_ASSIGN(WebView);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_WEB_VIEW_H_
