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

#include <QPointer>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QtGlobal>
#include <QUrl>

#include "qt/core/glue/oxide_qt_web_view_adapter.h"

#include "oxideqquicknavigationhistory_p.h"

class OxideQNewViewRequest;
class OxideQQuickScriptMessageHandler;
class OxideQQuickWebContext;
class OxideQQuickWebContextPrivate;
class OxideQQuickWebView;

QT_BEGIN_NAMESPACE
class QQmlComponent;
template <typename T> class QQmlListProperty;
class QQuickItem;
class QQuickWindow;
QT_END_NAMESPACE

namespace oxide {
namespace qt {
class CompositorFrameHandle;
}
}

class OxideQQuickWebViewPrivate final : public oxide::qt::WebViewAdapter {
  Q_DECLARE_PUBLIC(OxideQQuickWebView)

 public:
  ~OxideQQuickWebViewPrivate();

  static OxideQQuickWebViewPrivate* get(OxideQQuickWebView* web_view);

  void addAttachedPropertyTo(QObject* object);

 private:
  friend class UpdatePaintNodeScope;

  OxideQQuickWebViewPrivate(OxideQQuickWebView* view);

  oxide::qt::WebPopupMenuDelegate* CreateWebPopupMenuDelegate() final;
  oxide::qt::JavaScriptDialogDelegate* CreateJavaScriptDialogDelegate(
      oxide::qt::JavaScriptDialogDelegate::Type type) final;
  oxide::qt::JavaScriptDialogDelegate* CreateBeforeUnloadDialogDelegate() final;
  oxide::qt::FilePickerDelegate* CreateFilePickerDelegate() final;

  void OnInitialized() final;

  void URLChanged() final;
  void TitleChanged() final;
  void IconChanged(QUrl icon) final;
  void CommandsUpdated() final;

  void LoadingChanged() final;
  void LoadProgressChanged(double progress) final;

  void LoadEvent(OxideQLoadEvent* event) final;
  
  void NavigationEntryCommitted() final;
  void NavigationListPruned(bool from_front, int count) final;
  void NavigationEntryChanged(int index) final;

  oxide::qt::WebFrameAdapter* CreateWebFrame() final;

  QScreen* GetScreen() const final;
  QRect GetViewBoundsPix() const final;
  bool IsVisible() const final;
  bool HasFocus() const final;

  void AddMessageToConsole(int level,
			   const QString& message,
			   int line_no,
			   const QString& source_id) final;

  void ToggleFullscreenMode(bool enter) final;

  void OnWebPreferencesReplaced() final;

  void FrameAdded(oxide::qt::WebFrameAdapter* frame) final;
  void FrameRemoved(oxide::qt::WebFrameAdapter* frame) final;

  bool CanCreateWindows() const final;

  void UpdateCursor(const QCursor& cursor) final;

  void NavigationRequested(OxideQNavigationRequest* request) final;
  void NewViewRequested(OxideQNewViewRequest* request) final;

  void RequestGeolocationPermission(
      OxideQGeolocationPermissionRequest* request) final;

  void HandleUnhandledKeyboardEvent(QKeyEvent *event) final;

  void OnFrameMetadataUpdated(
      oxide::qt::FrameMetadataChangeFlags flags) final;

  void ScheduleUpdate() final;
  void EvictCurrentFrame() final;

  void SetInputMethodEnabled(bool enabled) final;

  void DownloadRequested(OxideQDownloadRequest* downloadRequest) final;

  void CertificateError(OxideQCertificateError* cert_error) final;
  void ContentBlocked() final;

  void PrepareToCloseResponse(bool proceed) final;
  void CloseRequested() final;

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

  void onWindowChanged(QQuickWindow* window);

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
  QSharedPointer<oxide::qt::CompositorFrameHandle> compositor_frame_handle_;

  bool using_old_load_event_signal_;

  struct ConstructProps {
    ConstructProps()
        : incognito(false) {}

    bool incognito;
    QPointer<OxideQQuickWebContext> context;
    QPointer<OxideQNewViewRequest> new_view_request;
  };

  QScopedPointer<ConstructProps> construct_props_;
};

#endif // _OXIDE_QT_QUICK_API_WEB_VIEW_P_P_H_
