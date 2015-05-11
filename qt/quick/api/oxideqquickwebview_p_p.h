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

#ifndef _OXIDE_QT_QUICK_API_WEB_VIEW_P_P_H_
#define _OXIDE_QT_QUICK_API_WEB_VIEW_P_P_H_

#include <QByteArray>
#include <QPointer>
#include <QScopedPointer>
#include <QtGlobal>
#include <QUrl>

#include "qt/core/glue/oxide_qt_web_view_proxy.h"
#include "qt/core/glue/oxide_qt_web_view_proxy_client.h"

#include "oxideqquicknavigationhistory_p.h"

class OxideQNewViewRequest;
class OxideQQuickLocationBarController;
class OxideQQuickScriptMessageHandler;
class OxideQQuickWebContext;
class OxideQQuickWebContextPrivate;
class OxideQQuickWebView;

QT_BEGIN_NAMESPACE
class QQmlComponent;
template <typename T> class QQmlListProperty;
class QQuickItem;
class QQuickWindow;
class QScreen;
QT_END_NAMESPACE

class OxideQQuickWebViewPrivate : public oxide::qt::WebViewProxyHandle,
                                  public oxide::qt::WebViewProxyClient {
  Q_DECLARE_PUBLIC(OxideQQuickWebView)
  OXIDE_Q_DECL_PROXY_HANDLE_CONVERTER(OxideQQuickWebView, oxide::qt::WebViewProxyHandle)
 public:
  ~OxideQQuickWebViewPrivate();

  static OxideQQuickWebViewPrivate* get(OxideQQuickWebView* web_view);

  void addAttachedPropertyTo(QObject* object);

  // XXX(chrisccoulson): Add LocationBarControllerProxy and remove these
  int locationBarHeight();
  void setLocationBarHeight(int height);
  oxide::qt::LocationBarMode locationBarMode() const;
  void setLocationBarMode(oxide::qt::LocationBarMode mode);
  bool locationBarAnimated() const;
  void setLocationBarAnimated(bool animated);
  int locationBarOffsetPix();
  int locationBarContentOffsetPix();
  void locationBarShow(bool animate);
  void locationBarHide(bool animate);

  // XXX(chrisccoulson): Add NavigationControllerProxy and remove these
  int getNavigationEntryCount() const;
  int getNavigationCurrentEntryIndex() const;
  void setNavigationCurrentEntryIndex(int index);
  int getNavigationEntryUniqueID(int index) const;
  QUrl getNavigationEntryUrl(int index) const;
  QString getNavigationEntryTitle(int index) const;
  QDateTime getNavigationEntryTimestamp(int index) const;

 private:
  friend class UpdatePaintNodeScope;

  OxideQQuickWebViewPrivate(OxideQQuickWebView* view);

  // oxide::qt::WebViewProxyClient implementation
  void Initialized() override;
  QObject* GetApiHandle() override;
  oxide::qt::WebPopupMenuProxy* CreateWebPopupMenu(
      oxide::qt::WebPopupMenuProxyClient* client) override;
  oxide::qt::JavaScriptDialogProxy* CreateJavaScriptDialog(
      oxide::qt::JavaScriptDialogProxyClient::Type type,
      oxide::qt::JavaScriptDialogProxyClient* client) override;
  oxide::qt::JavaScriptDialogProxy* CreateBeforeUnloadDialog(
      oxide::qt::JavaScriptDialogProxyClient* client) override;
  oxide::qt::FilePickerProxy* CreateFilePicker(
      oxide::qt::FilePickerProxyClient* client) override;
  void WebProcessStatusChanged() override;
  void URLChanged() override;
  void TitleChanged() override;
  void IconChanged(QUrl icon) override;
  void CommandsUpdated() override;
  void LoadingChanged() override;
  void LoadProgressChanged(double progress) override;
  void LoadEvent(OxideQLoadEvent* event) override;
  void NavigationEntryCommitted() override;
  void NavigationListPruned(bool from_front, int count) override;
  void NavigationEntryChanged(int index) override;
  oxide::qt::WebFrameProxyHandle* CreateWebFrame(
      oxide::qt::WebFrameProxy* proxy) override;
  QScreen* GetScreen() const override;
  QRect GetViewBoundsPix() const override;
  bool IsVisible() const override;
  bool HasFocus() const override;
  void AddMessageToConsole(int level,
                           const QString& message,
                           int line_no,
                           const QString& source_id) override;
  void ToggleFullscreenMode(bool enter) override;
  void WebPreferencesReplaced() override;
  void FrameAdded(oxide::qt::WebFrameProxyHandle* frame) override;
  void FrameRemoved(oxide::qt::WebFrameProxyHandle* frame) override;
  bool CanCreateWindows() const override;
  void UpdateCursor(const QCursor& cursor) override;
  void NavigationRequested(OxideQNavigationRequest* request) override;
  void NewViewRequested(OxideQNewViewRequest* request) override;
  void RequestGeolocationPermission(
      OxideQGeolocationPermissionRequest* request) override;
  void HandleUnhandledKeyboardEvent(QKeyEvent *event) override;
  void FrameMetadataUpdated(
      oxide::qt::FrameMetadataChangeFlags flags) override;
  void ScheduleUpdate() override;
  void EvictCurrentFrame() override;
  void SetInputMethodEnabled(bool enabled) override;
  void DownloadRequested(OxideQDownloadRequest* download_request) override;
  void CertificateError(OxideQCertificateError* cert_error) override;
  void ContentBlocked() override;
  void PrepareToCloseResponse(bool proceed) override;
  void CloseRequested() override;

  oxide::qt::WebViewProxy* proxy() const {
    return oxide::qt::WebViewProxyHandle::proxy();
  }

  void completeConstruction();

  static void messageHandler_append(
      QQmlListProperty<OxideQQuickScriptMessageHandler>* prop,
      OxideQQuickScriptMessageHandler* message_handler);
  static int messageHandler_count(
      QQmlListProperty<OxideQQuickScriptMessageHandler>* prop);
  static OxideQQuickScriptMessageHandler* messageHandler_at(
      QQmlListProperty<OxideQQuickScriptMessageHandler>* prop,
      int index);
  static void messageHandler_clear(
      QQmlListProperty<OxideQQuickScriptMessageHandler>* prop);

  void contextConstructed();
  void contextDestroyed();
  void attachContextSignals(OxideQQuickWebContextPrivate* context);
  void detachContextSignals(OxideQQuickWebContextPrivate* context);

  void didUpdatePaintNode();

  void screenChanged(QScreen* screen);
  void screenChangedHelper(QScreen* screen);
  void windowChangedHelper(QQuickWindow* window);

  void screenGeometryChanged(const QRect&);
  void screenOrientationChanged(Qt::ScreenOrientation);

  QPointer<QScreen> screen_;
  QPointer<QQuickWindow> window_;

  bool constructed_;
  int load_progress_;
  QUrl icon_;
  OxideQQuickNavigationHistory navigation_history_;
  QQmlComponent* popup_menu_;
  QQmlComponent* alert_dialog_;
  QQmlComponent* confirm_dialog_;
  QQmlComponent* prompt_dialog_;
  QQmlComponent* before_unload_dialog_;
  QQmlComponent* file_picker_;

  QQuickItem* input_area_;

  bool received_new_compositor_frame_;
  bool frame_evicted_;
  oxide::qt::CompositorFrameHandle::Type last_composited_frame_type_;

  bool using_old_load_event_signal_;

  struct ConstructProps {
    ConstructProps()
        : incognito(false)
        , restore_type(oxide::qt::RESTORE_LAST_SESSION_EXITED_CLEANLY) {}

    bool incognito;
    QPointer<OxideQQuickWebContext> context;
    QPointer<OxideQNewViewRequest> new_view_request;
    QByteArray restore_state;
    oxide::qt::RestoreType restore_type;
  };

  QScopedPointer<ConstructProps> construct_props_;

  QScopedPointer<OxideQQuickLocationBarController> location_bar_controller_;
};

#endif // _OXIDE_QT_QUICK_API_WEB_VIEW_P_P_H_
