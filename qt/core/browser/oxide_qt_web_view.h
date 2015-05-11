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
#include <QObject>
#include <QSharedPointer>
#include <QtGlobal>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"

#include "qt/core/common/oxide_qt_event_utils.h"
#include "qt/core/glue/oxide_qt_web_view_proxy.h"
#include "shared/browser/oxide_javascript_dialog_manager.h"
#include "shared/browser/oxide_web_view.h"

QT_BEGIN_NAMESPACE
class QFocusEvent;
class QInputMethodEvent;
class QKeyEvent;
class QMouseEvent;
class QWheelEvent;
QT_END_NAMESPACE

class OxideQSecurityStatus;

namespace oxide {
namespace qt {

class CompositorFrameHandle;
class WebContext;
class WebViewProxyClient;

class WebView : public QObject,
                public oxide::WebView,
                public WebViewProxy {
  Q_OBJECT

 public:
  WebView(WebViewProxyClient* client);
  ~WebView();

  static WebView* FromProxyHandle(WebViewProxyHandle* handle);

  WebContext* GetContext() const;

  void FrameAdded(oxide::WebFrame* frame);
  void FrameRemoved(oxide::WebFrame* frame);

 private Q_SLOTS:
  void OnInputPanelVisibilityChanged();

 private:
  float GetDeviceScaleFactor() const;

  bool ShouldShowInputPanel() const;
  bool ShouldHideInputPanel() const;
  void SetInputPanelVisibility(bool visible);

  // XXX(chrisccoulson): Move this in to oxide::WebView and the actual
  // unpickling to Init
  void RestoreState(qt::RestoreType type, const QByteArray& state);

  void EnsurePreferences();

  // WebView implementation
  void Init(oxide::WebView::Params* params) override;

  blink::WebScreenInfo GetScreenInfo() const override;
  gfx::Rect GetViewBoundsPix() const override;
  bool IsVisible() const override;
  bool HasFocus() const override;
  bool IsInputPanelVisible() const override;

  oxide::JavaScriptDialog* CreateJavaScriptDialog(
      content::JavaScriptMessageType javascript_message_type) override;
  oxide::JavaScriptDialog* CreateBeforeUnloadDialog() override;

  bool CanCreateWindows() const override;

  size_t GetScriptMessageHandlerCount() const override;
  const oxide::ScriptMessageHandler* GetScriptMessageHandlerAt(
      size_t index) const override;

  void OnCrashedStatusChanged() override;

  void OnURLChanged() override;
  void OnTitleChanged() override;
  void OnIconChanged(const GURL& icon) override;
  void OnCommandsUpdated() override;

  void OnLoadingChanged() override;
  void OnLoadProgressChanged(double progress) override;

  void OnLoadStarted(const GURL& validated_url) override;
  void OnLoadRedirected(const GURL& url,
                        const GURL& original_url) override;
  void OnLoadCommitted(const GURL& url,
                       bool is_error_page) override;
  void OnLoadStopped(const GURL& validated_url) override;
  void OnLoadFailed(const GURL& validated_url,
                    int error_code,
                    const std::string& error_description) override;
  void OnLoadSucceeded(const GURL& validated_url) override;

  void OnNavigationEntryCommitted() override;
  void OnNavigationListPruned(bool from_front, int count) override;
  void OnNavigationEntryChanged(int index) override;

  bool OnAddMessageToConsole(int32 level,
                             const base::string16& message,
                             int32 line_no,
                             const base::string16& source_id) override;

  void OnToggleFullscreenMode(bool enter) override;

  void OnWebPreferencesDestroyed() override;

  void OnRequestGeolocationPermission(
      scoped_ptr<oxide::SimplePermissionRequest> request) override;

  void OnUnhandledKeyboardEvent(
      const content::NativeWebKeyboardEvent& event) override;

  void OnFrameMetadataUpdated(const cc::CompositorFrameMetadata& old) override;

  void OnDownloadRequested(const GURL& url,
			   const std::string& mimeType,
			   const bool shouldPrompt,
			   const base::string16& suggestedFilename,
			   const std::string& cookies,
			   const std::string& referrer) override;

  bool ShouldHandleNavigation(const GURL& url,
                              WindowOpenDisposition disposition,
                              bool user_gesture) override;

  oxide::WebFrame* CreateWebFrame(
      content::RenderFrameHost* render_frame_host) override;
  oxide::WebPopupMenu* CreatePopupMenu(content::RenderFrameHost* rfh) override;

  oxide::WebView* CreateNewWebView(const gfx::Rect& initial_pos,
                                   WindowOpenDisposition disposition) override;

  oxide::FilePicker* CreateFilePicker(content::RenderViewHost* rvh) override;

  void OnSwapCompositorFrame() override;
  void OnEvictCurrentFrame() override;

  void OnTextInputStateChanged() override;
  void OnFocusedNodeChanged() override;
  void OnSelectionBoundsChanged() override;
  void OnImeCancelComposition() override;
  void OnSelectionChanged() override;

  void OnUpdateCursor(const content::WebCursor& cursor) override;

  void OnSecurityStatusChanged(const oxide::SecurityStatus& old) override;
  void OnCertificateError(scoped_ptr<oxide::CertificateError> error) override;
  void OnContentBlocked() override;

  void OnPrepareToCloseResponse(bool proceed) override;
  void OnCloseRequested() override;

  // WebViewProxy implementation
  void init(bool incognito,
            WebContextProxyHandle* context,
            OxideQNewViewRequest* new_view_request,
            const QByteArray& restore_state,
            qt::RestoreType restore_type) override;

  QUrl url() const override;
  void setUrl(const QUrl& url) override;

  QString title() const override;

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
  void handleInputMethodEvent(QInputMethodEvent* event) override;
  void handleKeyEvent(QKeyEvent* event) override;
  void handleMouseEvent(QMouseEvent* event) override;
  void handleTouchEvent(QTouchEvent* event) override;
  void handleWheelEvent(QWheelEvent* event) override;

  QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

  void goBack() override;
  void goForward() override;
  void stop() override;
  void reload() override;

  void loadHtml(const QString& html, const QUrl& base_url) override;

  QList<ScriptMessageHandlerProxyHandle*>& messageHandlers() override;

  bool isInitialized() const override;

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

  OxideQSecurityStatus* securityStatus() override;

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

  WebViewProxyClient* client_;

  bool has_input_method_state_;

  scoped_ptr<OxideQSecurityStatus> qsecurity_status_;
  QList<ScriptMessageHandlerProxyHandle*> message_handlers_;

  UITouchEventFactory touch_event_factory_;

  QSharedPointer<CompositorFrameHandle> compositor_frame_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebView);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_WEB_VIEW_H_
