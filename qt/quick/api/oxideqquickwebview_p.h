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

#ifndef _OXIDE_QT_QUICK_API_WEB_VIEW_P_P_H_
#define _OXIDE_QT_QUICK_API_WEB_VIEW_P_P_H_

#include <QPointer>
#include <QScopedPointer>
#include <QtGlobal>
#include <QUrl>

#include "qt/core/api/oxideqfindcontroller.h"
#include "qt/core/api/oxideqsecuritystatus.h"
#include "qt/core/glue/oxide_qt_web_view_proxy.h"
#include "qt/core/glue/oxide_qt_web_view_proxy_client.h"

#include "qt/quick/api/oxideqquicknavigationhistory_p.h"
#include "qt/quick/api/oxideqquickwebview.h"

class OxideQNewViewRequest;
class OxideQQuickLocationBarController;
class OxideQQuickScriptMessageHandler;
class OxideQQuickWebContextPrivate;
class OxideQQuickWebView;

QT_BEGIN_NAMESPACE
class QQmlComponent;
template <typename T> class QQmlListProperty;
class QQuickItem;
QT_END_NAMESPACE

namespace oxide {
namespace qquick {
class ContentsView;
}
}

class OxideQQuickWebViewPrivate : public oxide::qt::WebViewProxyClient {
  Q_DECLARE_PUBLIC(OxideQQuickWebView)

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
  int locationBarOffset();
  int locationBarContentOffset();
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
  OxideQQuickWebViewPrivate(OxideQQuickWebView* view);

  // oxide::qt::WebViewProxyClient implementation
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
  void FaviconChanged() override;
  void CommandsUpdated() override;
  void LoadingChanged() override;
  void LoadProgressChanged(double progress) override;
  void LoadEvent(const OxideQLoadEvent& event) override;
  void NavigationEntryCommitted() override;
  void NavigationListPruned(bool from_front, int count) override;
  void NavigationEntryChanged(int index) override;
  void CreateWebFrame(oxide::qt::WebFrameProxy* proxy) override;
  void AddMessageToConsole(int level,
                           const QString& message,
                           int line_no,
                           const QString& source_id) override;
  void ToggleFullscreenMode(bool enter) override;
  void WebPreferencesReplaced() override;
  void FrameRemoved(QObject* frame) override;
  bool CanCreateWindows() const override;
  void NavigationRequested(OxideQNavigationRequest* request) override;
  void NewViewRequested(OxideQNewViewRequest* request) override;
  void RequestGeolocationPermission(
      OxideQGeolocationPermissionRequest* request) override;
  void RequestMediaAccessPermission(
      OxideQMediaAccessPermissionRequest* request) override;
  void RequestNotificationPermission(
      OxideQPermissionRequest* request) override;
  void FrameMetadataUpdated(
      oxide::qt::FrameMetadataChangeFlags flags) override;
  void DownloadRequested(
      const OxideQDownloadRequest& download_request) override;
  void HttpAuthenticationRequested(
      OxideQHttpAuthenticationRequest* authentication_request) override;
  void CertificateError(OxideQCertificateError* cert_error) override;
  void ContentBlocked() override;
  void PrepareToCloseResponse(bool proceed) override;
  void CloseRequested() override;
  void TargetURLChanged() override;
  void OnEditingCapabilitiesChanged() override;

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

  QList<QObject*>& messageHandlers();

  QObject* contextHandle() const;

  void contextConstructed();
  void contextDestroyed();
  void attachContextSignals(OxideQQuickWebContextPrivate* context);
  void detachContextSignals(OxideQQuickWebContextPrivate* context);

  OxideQQuickWebView* q_ptr;

  QScopedPointer<oxide::qquick::ContentsView> contents_view_;

  QScopedPointer<oxide::qt::WebViewProxy> proxy_;

  bool constructed_;
  int load_progress_;

  OxideQQuickNavigationHistory navigation_history_;
  QScopedPointer<OxideQSecurityStatus> security_status_;
  QScopedPointer<OxideQFindController> find_controller_;

  QQmlComponent* alert_dialog_;
  QQmlComponent* confirm_dialog_;
  QQmlComponent* prompt_dialog_;
  QQmlComponent* before_unload_dialog_;
  QQmlComponent* file_picker_;

  bool using_old_load_event_signal_;

  struct ConstructProps;
  QScopedPointer<ConstructProps> construct_props_;

  QScopedPointer<OxideQQuickLocationBarController> location_bar_controller_;
};

#endif // _OXIDE_QT_QUICK_API_WEB_VIEW_P_P_H_
