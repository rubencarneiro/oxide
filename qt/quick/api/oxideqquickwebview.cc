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

#include "oxideqquickwebview.h"
#include "oxideqquickwebview_p.h"

#include <QByteArray>
#include <QEvent>
#include <QFlags>
#include <QHoverEvent>
#include <QImage>
#include <QKeyEvent>
#include <QMetaMethod>
#include <QMouseEvent>
#include <QPointF>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QRect>
#include <QRectF>
#include <QSGNode>
#include <QSizeF>
#include <QSize>
#include <Qt>

#include "qt/core/api/oxideqcertificateerror.h"
#include "qt/core/api/oxideqfindcontroller.h"
#include "qt/core/api/oxideqfindcontroller_p.h"
#include "qt/core/api/oxideqglobal.h"
#include "qt/core/api/oxideqglobal_p.h"
#include "qt/core/api/oxideqhttpauthenticationrequest.h"
#include "qt/core/api/oxideqloadevent.h"
#include "qt/core/api/oxideqnewviewrequest.h"
#include "qt/core/api/oxideqpermissionrequest.h"
#include "qt/core/api/oxideqsecuritystatus.h"
#include "qt/core/api/oxideqsecuritystatus_p.h"
#include "qt/core/api/oxideqwebpreferences.h"
#include "qt/core/glue/macros.h"
#include "qt/quick/auxiliary_ui_factory.h"
#include "qt/quick/oxide_qquick_alert_dialog.h"
#include "qt/quick/oxide_qquick_before_unload_dialog.h"
#include "qt/quick/oxide_qquick_confirm_dialog.h"
#include "qt/quick/oxide_qquick_contents_view.h"
#include "qt/quick/oxide_qquick_file_picker.h"
#include "qt/quick/oxide_qquick_init.h"
#include "qt/quick/oxide_qquick_prompt_dialog.h"
#include "qt/quick/qquick_web_context_menu.h"

#include "oxideqquicklocationbarcontroller.h"
#include "oxideqquicklocationbarcontroller_p.h"
#include "oxideqquicknavigationhistory_p.h"
#include "oxideqquickscriptmessagehandler.h"
#include "oxideqquickscriptmessagehandler_p.h"
#include "oxideqquickwebcontext.h"
#include "oxideqquickwebcontext_p.h"
#include "oxideqquickwebframe.h"
#include "oxideqquickwebframe_p.h"

namespace {

QEvent::Type GetPrepareToCloseBypassEventType() {
  static int g_event_type = QEvent::registerEventType();
  return QEvent::Type(g_event_type);
}

oxide::qt::RestoreType ToInternalRestoreType(
    OxideQQuickWebView::RestoreType type) {
  switch (type) {
    case OxideQQuickWebView::RestoreCurrentSession:
      return oxide::qt::RESTORE_CURRENT_SESSION;
    case OxideQQuickWebView::RestoreLastSessionExitedCleanly:
      return oxide::qt::RESTORE_LAST_SESSION_EXITED_CLEANLY;
    case OxideQQuickWebView::RestoreLastSessionCrashed:
      return oxide::qt::RESTORE_LAST_SESSION_CRASHED;
  }
}

}

OxideQQuickWebViewAttached::OxideQQuickWebViewAttached(QObject* parent) :
    QObject(parent),
    view_(nullptr) {}

OxideQQuickWebViewAttached::~OxideQQuickWebViewAttached() {}

OxideQQuickWebView* OxideQQuickWebViewAttached::view() const {
  return view_;
}

void OxideQQuickWebViewAttached::setView(OxideQQuickWebView* view) {
  view_ = view;
}

struct OxideQQuickWebViewPrivate::ConstructProps {
  ConstructProps()
      : incognito(false),
        context(nullptr),
        restore_type(oxide::qt::RESTORE_LAST_SESSION_EXITED_CLEANLY),
        load_html(false),
        fullscreen(false) {}

  bool incognito;
  QPointer<OxideQQuickWebContext> context;
  QPointer<OxideQNewViewRequest> new_view_request;
  QByteArray restore_state;
  oxide::qt::RestoreType restore_type;
  QList<QObject*> message_handlers;
  bool load_html;
  QUrl url;
  QString html;
  bool fullscreen;
  QPointer<OxideQWebPreferences> preferences;
};

OxideQQuickWebViewPrivate::OxideQQuickWebViewPrivate(
    OxideQQuickWebView* view,
    std::unique_ptr<oxide::qquick::AuxiliaryUIFactory> aux_ui_factory)
    : q_ptr(view),
      aux_ui_factory_(std::move(aux_ui_factory)),
      constructed_(false),
      load_progress_(0),
      security_status_(OxideQSecurityStatusPrivate::Create()),
      find_controller_(OxideQFindControllerPrivate::Create()),
      context_menu_(nullptr),
      alert_dialog_(nullptr),
      confirm_dialog_(nullptr),
      prompt_dialog_(nullptr),
      before_unload_dialog_(nullptr),
      file_picker_(nullptr),
      using_old_load_event_signal_(false),
      construct_props_(new ConstructProps()) {}

std::unique_ptr<oxide::qt::WebContextMenu>
OxideQQuickWebViewPrivate::CreateWebContextMenu(
    const oxide::qt::WebContextMenuParams& params,
    const std::vector<oxide::qt::MenuItem>& items,
    oxide::qt::WebContextMenuClient* client) {
  Q_Q(OxideQQuickWebView);

  if (aux_ui_factory_) {
    return aux_ui_factory_->CreateWebContextMenu(params, items, client);
  }

  if (!context_menu_) {
    return nullptr;
  }

  return std::unique_ptr<oxide::qt::WebContextMenu>(
      new oxide::qquick::WebContextMenu(q, context_menu_, params, client));
}

oxide::qt::JavaScriptDialogProxy*
OxideQQuickWebViewPrivate::CreateJavaScriptDialog(
    oxide::qt::JavaScriptDialogProxyClient::Type type,
    oxide::qt::JavaScriptDialogProxyClient* client) {
  Q_Q(OxideQQuickWebView);

  switch (type) {
  case oxide::qt::JavaScriptDialogProxyClient::TypeAlert:
    return new oxide::qquick::AlertDialog(q, client);
  case oxide::qt::JavaScriptDialogProxyClient::TypeConfirm:
    return new oxide::qquick::ConfirmDialog(q, client);
  case oxide::qt::JavaScriptDialogProxyClient::TypePrompt:
    return new oxide::qquick::PromptDialog(q, client);
  default:
    Q_UNREACHABLE();
  }
}

oxide::qt::JavaScriptDialogProxy*
OxideQQuickWebViewPrivate::CreateBeforeUnloadDialog(
    oxide::qt::JavaScriptDialogProxyClient* client) {
  Q_Q(OxideQQuickWebView);

  return new oxide::qquick::BeforeUnloadDialog(q, client);
}

oxide::qt::FilePickerProxy* OxideQQuickWebViewPrivate::CreateFilePicker(
    oxide::qt::FilePickerProxyClient* client) {
  Q_Q(OxideQQuickWebView);

  return new oxide::qquick::FilePicker(q, client);
}

void OxideQQuickWebViewPrivate::WebProcessStatusChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->webProcessStatusChanged();
}

void OxideQQuickWebViewPrivate::URLChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->urlChanged();
}

void OxideQQuickWebViewPrivate::TitleChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->titleChanged();
}

void OxideQQuickWebViewPrivate::FaviconChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->iconChanged();
}

void OxideQQuickWebViewPrivate::LoadingChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->loadingStateChanged();
}

void OxideQQuickWebViewPrivate::LoadProgressChanged(double progress) {
  Q_Q(OxideQQuickWebView);

  load_progress_ = progress * 100;
  emit q->loadProgressChanged();
}

void OxideQQuickWebViewPrivate::LoadEvent(const OxideQLoadEvent& event) {
  Q_Q(OxideQQuickWebView);

  emit q->loadEvent(event);

  // The deprecated signal doesn't get TypeCommitted or TypeRedirected
  if (!using_old_load_event_signal_ ||
      event.type() == OxideQLoadEvent::TypeCommitted ||
      event.type() == OxideQLoadEvent::TypeRedirected) {
    return;
  }

  emit q->loadingChanged(event);
}

void OxideQQuickWebViewPrivate::CreateWebFrame(
    oxide::qt::WebFrameProxy* proxy) {
  Q_Q(OxideQQuickWebView);

  OxideQQuickWebFrame* frame = OxideQQuickWebFramePrivate::create(proxy);
  QQmlEngine::setObjectOwnership(frame, QQmlEngine::CppOwnership);

  emit q->frameAdded(frame);
}

void OxideQQuickWebViewPrivate::AddMessageToConsole(
    int level,
    const QString& message,
    int line_no,
    const QString& source_id) {
  Q_Q(OxideQQuickWebView);

  OxideQQuickWebView::LogMessageSeverityLevel oxideLevel =
    OxideQQuickWebView::LogSeverityInfo;
  if (level >= 0 && level <= OxideQQuickWebView::LogSeverityFatal) {
    oxideLevel = static_cast<OxideQQuickWebView::LogMessageSeverityLevel>(level);
  }
  emit q->javaScriptConsoleMessage(
      oxideLevel,
      message,
      line_no,
      source_id);
}

void OxideQQuickWebViewPrivate::ToggleFullscreenMode(bool enter) {
  Q_Q(OxideQQuickWebView);

  emit q->fullscreenRequested(enter);
}

void OxideQQuickWebViewPrivate::FrameRemoved(QObject* frame) {
  Q_Q(OxideQQuickWebView);

  emit q->frameRemoved(qobject_cast<OxideQQuickWebFrame*>(frame));
}

bool OxideQQuickWebViewPrivate::CanCreateWindows() const {
  Q_Q(const OxideQQuickWebView);

  // QObject::isSignalConnected doesn't work from here (it still indicates
  // true during the last disconnect)
  return q->receivers(SIGNAL(newViewRequested(OxideQNewViewRequest*))) > 0;
}

void OxideQQuickWebViewPrivate::NavigationRequested(
    OxideQNavigationRequest* request) {
  Q_Q(OxideQQuickWebView);

  emit q->navigationRequested(request);
}

void OxideQQuickWebViewPrivate::NewViewRequested(
    OxideQNewViewRequest* request) {
  Q_Q(OxideQQuickWebView);

  emit q->newViewRequested(request);
}

void OxideQQuickWebViewPrivate::RequestGeolocationPermission(
    OxideQGeolocationPermissionRequest* request) {
  Q_Q(OxideQQuickWebView);

  // Unlike other signals where the object ownership remains with the
  // callsite that initiates the signal, we want to transfer ownership of
  // GeolocationPermissionRequest to any signal handlers so that the
  // request can be completed asynchronously.
  //
  // Setting JavaScriptOwnership on the request, dispatching the signal and
  // hoping the QmlEngine will delete it isn't enough though, because it will
  // leak if there are no handlers. It's also unclear whether C++ slots should
  // delete it.
  //
  // Handlers could be C++ slots in addition to the QmlEngine and so we need
  // a way to safely share the object between slots - should a C++ slot
  // delete the object? What if the QmlEngine deletes it?
  //
  // In a C++ world, we could wrap it in a QSharedPointer or dispatch a
  // copyable type that manages shareable data underneath. We can't use
  // QSharedPointer with Qml though, because the engine will delete any object
  // without a parent when it goes out of scope, regardless of any shared
  // references, and QObject parents are incompatible with QSharedPointer.
  // Instead we transfer ownership to the QmlEngine now and dispatch a QJSValue.
  // This means that the request only has a single owner (the QmlEngine), which
  // won't delete it until all QJSValue's have gone out of scope

  QQmlEngine* engine = qmlEngine(q);
  if (!engine) {
    delete request;
    return;
  }

  {
    QJSValue val = engine->newQObject(request);
    if (!val.isQObject()) {
      delete request;
      return;
    }

    emit q->geolocationPermissionRequested(val);
  }

  engine->collectGarbage();
}

void OxideQQuickWebViewPrivate::RequestMediaAccessPermission(
    OxideQMediaAccessPermissionRequest* request) {
  Q_Q(OxideQQuickWebView);

  // See the comment in RequestGeolocationPermission

  QQmlEngine* engine = qmlEngine(q);
  if (!engine) {
    delete request;
    return;
  }

  {
    QJSValue val = engine->newQObject(request);
    if (!val.isQObject()) {
      delete request;
      return;
    }

    emit q->mediaAccessPermissionRequested(val);
  }

  engine->collectGarbage();

}

void OxideQQuickWebViewPrivate::RequestNotificationPermission(
    OxideQPermissionRequest* request) {
  Q_Q(OxideQQuickWebView);

  QQmlEngine* engine = qmlEngine(q);
  if (!engine) {
    delete request;
    return;
  }

  {
    QJSValue val = engine->newQObject(request);
    if (!val.isQObject()) {
      delete request;
      return;
    }

    emit q->notificationPermissionRequested(val);
  }

  engine->collectGarbage();
}

void OxideQQuickWebViewPrivate::HttpAuthenticationRequested(
    OxideQHttpAuthenticationRequest* authentication_request) {
  Q_Q(OxideQQuickWebView);

  // See the comment in RequestGeolocationPermission

  QQmlEngine* engine = qmlEngine(q);
  if (!engine) {
    delete authentication_request;
    return;
  }

  {
    QJSValue val = engine->newQObject(authentication_request);
    if (!val.isQObject()) {
      delete authentication_request;
      return;
    }

    emit q->httpAuthenticationRequested(val);
  }

  engine->collectGarbage();
}

void OxideQQuickWebViewPrivate::FrameMetadataUpdated(
    oxide::qt::FrameMetadataChangeFlags flags) {
  Q_Q(OxideQQuickWebView);

  QFlags<oxide::qt::FrameMetadataChangeFlags> f(flags);

  if (f.testFlag(oxide::qt::FRAME_METADATA_CHANGE_SCROLL_OFFSET)) {
    emit q->contentXChanged();
    emit q->contentYChanged();
  }
  if (f.testFlag(oxide::qt::FRAME_METADATA_CHANGE_CONTENT)) {
    emit q->contentWidthChanged();
    emit q->contentHeightChanged();
  }
  if (f.testFlag(oxide::qt::FRAME_METADATA_CHANGE_VIEWPORT)) {
    emit q->viewportWidthChanged();
    emit q->viewportHeightChanged();
  }
}

void OxideQQuickWebViewPrivate::DownloadRequested(
    const OxideQDownloadRequest& download_request) {
  Q_Q(OxideQQuickWebView);

  emit q->downloadRequested(download_request);
}

void OxideQQuickWebViewPrivate::CertificateError(
    std::unique_ptr<OxideQCertificateError> cert_error) {
  Q_Q(OxideQQuickWebView);

  // See the comment in RequestGeolocationPermission
  QQmlEngine* engine = qmlEngine(q);
  if (!engine) {
    return;
  }

  {
    QJSValue val = engine->newQObject(cert_error.get());
    if (!val.isQObject()) {
      return;
    }

    // |cert_error| is owned by the QML engine
    cert_error.release();

    emit q->certificateError(val);
  }

  engine->collectGarbage();
}

void OxideQQuickWebViewPrivate::ContentBlocked() {
  Q_Q(OxideQQuickWebView);

  emit q->blockedContentChanged();
}

void OxideQQuickWebViewPrivate::PrepareToCloseResponse(bool proceed) {
  Q_Q(OxideQQuickWebView);

  emit q->prepareToCloseResponse(proceed);
}

void OxideQQuickWebViewPrivate::CloseRequested() {
  Q_Q(OxideQQuickWebView);

  emit q->closeRequested();
}

void OxideQQuickWebViewPrivate::TargetURLChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->hoveredUrlChanged();
}

void OxideQQuickWebViewPrivate::OnEditingCapabilitiesChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->editingCapabilitiesChanged();
}

void OxideQQuickWebViewPrivate::ZoomLevelChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->zoomFactorChanged();
}

void OxideQQuickWebViewPrivate::completeConstruction() {
  Q_Q(OxideQQuickWebView);

  Q_ASSERT(construct_props_.data());

  if (construct_props_->new_view_request) {
    OxideQWebPreferences* initial_prefs = nullptr;
    if (!construct_props_->preferences) {
      initial_prefs = new OxideQWebPreferences(q);
      attachPreferencesSignals(initial_prefs);
    }

    proxy_.reset(oxide::qt::WebViewProxy::create(
        this, contents_view_.data(), q,
        construct_props_->new_view_request,
        initial_prefs));
  }

  if (!proxy_) {
    Q_ASSERT(construct_props_->context);

    if (!construct_props_->preferences) {
      construct_props_->preferences = new OxideQWebPreferences(q);
      attachPreferencesSignals(construct_props_->preferences);
    }

    construct_props_->new_view_request = nullptr;

    proxy_.reset(oxide::qt::WebViewProxy::create(
        this, contents_view_.data(), q,
        construct_props_->context,
        construct_props_->incognito,
        construct_props_->restore_state,
        construct_props_->restore_type));
  }

  OxideQQuickNavigationHistoryPrivate::get(&navigation_history_)->init(
      proxy_->webContentsID());
  OxideQFindControllerPrivate::get(find_controller_.data())->Init(
      proxy_->webContentsID());
  OxideQSecurityStatusPrivate::get(security_status_.data())->Init(
      proxy_->webContentsID());
  if (location_bar_controller_) {
    OxideQQuickLocationBarControllerPrivate::get(location_bar_controller_.get())
        ->init(proxy_->webContentsID());
  }

  if (construct_props_->preferences) {
    proxy_->setPreferences(construct_props_->preferences);
  }

  proxy_->messageHandlers().swap(construct_props_->message_handlers);

  if (!construct_props_->new_view_request) {
    if (construct_props_->load_html) {
      proxy_->loadHtml(construct_props_->html, construct_props_->url);
    } else if (!construct_props_->url.isEmpty()) {
      proxy_->setUrl(construct_props_->url);
    }
  }

  proxy_->setFullscreen(construct_props_->fullscreen);

  // Initialization created the root frame. This is the only time
  // this is emitted
  emit q->rootFrameChanged();

  if (construct_props_->incognito != proxy_->incognito()) {
    emit q->incognitoChanged();
  }
  if (construct_props_->context != proxy_->context()) {
    if (construct_props_->context) {
      detachContextSignals(
          OxideQQuickWebContextPrivate::get(construct_props_->context));
    }
    attachContextSignals(
        OxideQQuickWebContextPrivate::get(
          qobject_cast<OxideQQuickWebContext*>(proxy_->context())));
    emit q->contextChanged();
  }

  emit q->editingCapabilitiesChanged();

  construct_props_.reset();
}

// static
void OxideQQuickWebViewPrivate::messageHandler_append(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop,
    OxideQQuickScriptMessageHandler* message_handler) {
  if (!message_handler) {
    return;
  }

  OxideQQuickWebView* web_view =
      static_cast<OxideQQuickWebView* >(prop->object);

  web_view->addMessageHandler(message_handler);
}

// static
int OxideQQuickWebViewPrivate::messageHandler_count(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop) {
  OxideQQuickWebViewPrivate* p = OxideQQuickWebViewPrivate::get(
      static_cast<OxideQQuickWebView*>(prop->object));

  return p->messageHandlers().size();
}

// static
OxideQQuickScriptMessageHandler* OxideQQuickWebViewPrivate::messageHandler_at(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop,
    int index) {
  OxideQQuickWebViewPrivate* p = OxideQQuickWebViewPrivate::get(
      static_cast<OxideQQuickWebView*>(prop->object));

  if (index >= p->messageHandlers().size()) {
    return nullptr;
  }

  return qobject_cast<OxideQQuickScriptMessageHandler*>(
      p->messageHandlers().at(index));
}

// static
void OxideQQuickWebViewPrivate::messageHandler_clear(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop) {
  OxideQQuickWebView* web_view =
      static_cast<OxideQQuickWebView *>(prop->object);
  OxideQQuickWebViewPrivate* p =
      OxideQQuickWebViewPrivate::get(web_view);

  while (p->messageHandlers().size() > 0) {
    web_view->removeMessageHandler(
        qobject_cast<OxideQQuickScriptMessageHandler*>(
            p->messageHandlers().at(0)));
  }
}

QList<QObject*>& OxideQQuickWebViewPrivate::messageHandlers() {
  if (!proxy_) {
    return construct_props_->message_handlers;
  }

  return proxy_->messageHandlers();
}

QObject* OxideQQuickWebViewPrivate::contextHandle() const {
  if (!proxy_) {
    return construct_props_->context;
  }

  return proxy_->context();
}

void OxideQQuickWebViewPrivate::contextConstructed() {
  if (constructed_) {
    completeConstruction();
  }
}

void OxideQQuickWebViewPrivate::contextDestroyed() {
  Q_Q(OxideQQuickWebView);

  if (construct_props_) {
    construct_props_->context = nullptr;
  }

  // XXX: Our underlying BrowserContext lives on, so we're left in a
  // bit of a weird state here (WebView.context will return no context,
  // which is a lie)
  emit q->contextChanged();
}

void OxideQQuickWebViewPrivate::attachContextSignals(
    OxideQQuickWebContextPrivate* context) {
  Q_Q(OxideQQuickWebView);

  if (!context) {
    return;
  }

  QObject::connect(context, SIGNAL(destroyed()),
                   q, SLOT(contextDestroyed()));
  QObject::connect(context, SIGNAL(constructed()),
                   q, SLOT(contextConstructed()));
}

void OxideQQuickWebViewPrivate::detachContextSignals(
    OxideQQuickWebContextPrivate* context) {
  Q_Q(OxideQQuickWebView);

  if (!context) {
    return;
  }

  QObject::disconnect(context, SIGNAL(constructed()),
                      q, SLOT(contextConstructed()));
  QObject::disconnect(context, SIGNAL(destroyed()),
                      q, SLOT(contextDestroyed()));
}

void OxideQQuickWebViewPrivate::attachPreferencesSignals(
    OxideQWebPreferences* prefs) {
  Q_Q(OxideQQuickWebView);

  q->connect(prefs, SIGNAL(destroyed()), SLOT(preferencesDestroyed()));
}

void OxideQQuickWebViewPrivate::preferencesDestroyed() {
  Q_Q(OxideQQuickWebView);

  // At this point, the oxide::WebPreferences instance associated with the
  // deleted OxideQWebPreferences has been orphaned. Provide a fresh preferences
  // instance
  q->setPreferences(nullptr);
}

OxideQQuickWebViewPrivate::~OxideQQuickWebViewPrivate() {}

// static
OxideQQuickWebViewPrivate* OxideQQuickWebViewPrivate::get(
    OxideQQuickWebView* view) {
  return view->d_func();
}

void OxideQQuickWebViewPrivate::addAttachedPropertyTo(QObject* object) {
  Q_Q(OxideQQuickWebView);

  OxideQQuickWebViewAttached* attached =
      qobject_cast<OxideQQuickWebViewAttached *>(
        qmlAttachedPropertiesObject<OxideQQuickWebView>(object));
  attached->setView(q);
}

/*!
\class OxideQQuickWebView
\inmodule OxideQtQuick
\inheaderfile oxideqquickwebview.h

\brief Display web content in a QML scene
*/

/*!
\qmltype WebView
\inqmlmodule com.canonical.Oxide 1.0
\instantiates OxideQQuickWebView

\brief Display web content in a QML scene

WebView is the central class that applications use to embed web content in a QML
scene.

Applications can load web content by setting \l{url} or calling loadHtml. Load
progress can be observed by monitoring the \l{loading} property, listening to
events from loadEvent and observing the loadProgress property.

The WebView API contains basic navigation actions in the form of goBack,
goForward, \l{reload} and \l{stop}.

The security status of the WebView is provided via securityStatus. If a
certificate error is encountered when trying to establish a secure connection to
a site, then this is signalled to the application via certificateError. In some
cases, this provides the application with an opportunity to abort or continue
the connection.

WebView doesn't provide any auxilliary user interfaces (JS dialogs, context
menu etc) by itself. It is currently the application's responsibility to provide
these as QML components by setting the alertDialog, beforeUnloadDialog,
confirmDialog, contextMenu, filePicker, popupMenu and promptDialog properties.

A structure that represents the frames within a currently displayed document is
provided via the rootFrame property. This allows applications to send messages
to user scripts that it has injected in to them.

The WebView provides APIs to allow applications to implement session-restore
type functionality (currentState, restoreState and restoreType).

There is an API that allows applications to intercept content-initiated
navigations, which is useful for implementing web applications
(navigationRequested). There is also an API to allow web content to open new
web views (newViewRequested).

It is possible for applications to implement private browsing functionality by
creating WebViews with the \l{incognito} property set to true.

The API provides various signals to allow web content to request permission to
access various resources, such as location (via geolocationPermissionRequested),
media capture devices (via mediaAccessPermissionRequested) and the system's
notification API (via notificationPermissionRequested).
*/

/*!
\qmlsignal void WebView::loadEvent(LoadEvent event)

This signal is used to notify the application of load events in the main frame.
Please refer to the documentation for LoadEvent for more information about how
to use it.

\a{event} is owned by the QML engine and will be garbage collected when it goes
out of scope.
*/

/*!
\qmlsignal void WebView::frameAdded(WebFrame frame)

This signal is used to indicate that \a{frame} was added to the frame tree for
this webview.
*/

/*!
\qmlsignal void WebView::frameRemoved(WebFrame frame)

This signal is used to indicate that \a{frame} was removed from the frame tree
for this webview.
*/

/*!
\qmlsignal void WebView::fullscreenRequested(bool fullscreen)

This signal indicates that web content has requested to change the fullscreen
state.

If \a{fullscreen} is true, then this is a request to enter fullscreen mode. If
the application permits this then it should make its window fullscreen, hide all
of its UI, and respond by setting the \l{fullscreen} property to true.

If \a{fullscreen} is false, then this is a request to exit fullscreen mode. In
this case, the application should un-fullscreen its window, unhide all of its
UI and respond by setting the \l{fullscreen} property to false.
*/

/*!
\qmlsignal void WebView::navigationRequested(NavigationRequest request)

This signal indicates that web content would like to navigate this webview.
Please refer to the documentation for NavigationRequest for more information
about how to use it.

If NavigationRequest::disposition is set to something other than
\e{NavigationRequest::DispositionCurrentTab}, then this is part of a request to
open a new webview. If the application allows this request, then
newViewRequested will be signalled afterwards to tell the application when to
create a new webview.

The application must respond to this synchronously.
*/

/*!
\qmlsignal void WebView::newViewRequested(NewViewRequest request)

This signal indicates that the application should create a new webview, either
as a result of web content calling \e{window.open()} or a link click with
modifier keys pressed. Please refer to the documentation for NewViewRequest for
more information about how to use it.

The application must respond to this by constructing a new WebView, and passing
\a{request} to the \l{request} property during construction.

If the application doesn't connect to this signal, then all requests to open a
new webview will be converted to navigations in this webview.

\note Applications that support creating new webviews must also connect to
closeRequested.
*/

/*!
\qmlsignal void WebView::geolocationPermissionRequested(GeolocationPermissionRequest request)

This signal indicates that the page inside this webview would like to access the
current location. Please refer to the documentation for PermissionRequest for
more information about how to use it.

The ownership of \a{request} is passed to the QML engine. This means that
applications can respond to this asynchronously. \a{request} will be garbage
collected when it goes out of scope.
*/

/*!
\qmlsignal void WebView::mediaAccessPermissionRequested(MediaAccessPermissionRequest request)
\since OxideQt 1.8

This signal indicates that the page inside this webview would like to access
media capture devices because of a call to \e{MediaDevices.getUserMedia()}.
Please refer to the documentation for MediaAccessPermissionRequest for more
information about how to use it.

The ownership of \a{request} is passed to the QML engine. This means that
applications can respond to this asynchronously. \a{request} will be garbage
collected when it goes out of scope.
*/

/*!
\qmlsignal void WebView::notificationPermissionRequested(PermissionRequest request)
\since OxideQt 1.11

This signal indicates that the page inside this webview would like to use the
notification API. Please refer to the documentation for PermissionRequest for
more information about how to use it.

Note that PermissionRequest::embedder behaves differently for notification
permission requests - it will be equal to PermissionRequest::origin.

The ownership of \a{request} is passed to the QML engine. This means that
applications can respond to this asynchronously. \a{request} will be garbage
collected when it goes out of scope.
*/

/*!
\qmlsignal void WebView::javaScriptConsoleMessage(emumeration level,
                                                  string message,
                                                  int lineNumber,
                                                  string sourceId)

This signal indicates that the web content inside this webview triggered a
console message, either via a call to \e{console.log()} or because of some
message logged inside Blink.

\a{message} provides the actual message. The source of the message is indicated
by \a{sourceId} and \a{lineNumber}.

\a{level} indicates the severity level of the message. It can be one of the
following values:

\value WebView.LogSeverityVerbose

\value WebView.LogSeverityInfo

\value WebView.LogSeverityWarning

\value WebView.LogSeverityError
*/

/*!
\qmlsignal void WebView::downloadRequested(DownloadRequest request)

This signal is a request for the application to download the resource specified
by \a{request}. Please refer to the documentation for DownloadRequest for
more information about how to use it.

\a{request} is owned by the QML engine and will be garbage collected when it
goes out of scope.
*/

/*!
\qmlsignal void WebView::certificateError(CertificateError error)
\since OxideQt 1.2

This signal is emitted when an error is encountered with the X.509 certificate
provided by a server when trying to establish a secure connection. Please refer
to the documentation for CertificateError for more information about how to use
it.

The ownership of \a{error} is passed to the QML engine. This means that
applications can respond to this asynchronously for errors that are overridable.
*/

/*!
\qmlsignal void WebView::prepareToCloseResponse(bool proceed)
\since OxideQt 1.4

This is the response to a call to prepareToClose. \a{proceed} is a hint that
indicates whether the application should proceed to delete the webview.

If \a{proceed} is false, then this means that the site caused a beforeunload
dialog to appear and the user declined the attempt to leave the page. This is a
hint that the application shouldn't proceed to delete the webview.
*/

/*!
\qmlsignal void WebView::closeRequested()
\since OxideQt 1.4

This signal is emitted when a web page calls \e{window.close()}. As calling
\e{window.close()} is destructive, the application must respond to this by
deleting the webview.

\note Web pages can only delete windows that they created, unless
WebPreferences::allowScriptsToCloseWindows is set to true.
*/

/*!
\qmlsignal void WebView::httpAuthenticationRequested(HttpAuthenticationRequest request);
\since OxideQt 1.9

This signal is a request for HTTP authentication credentials. Please refer to
the documentation for HttpAuthenticationRequest for more information about how
to use it.
*/

/*!
\qmlsignal void WebView::loadingStateChanged()
\since OxideQt 1.3

This signal is emitted whenever the value of \l{loading} changes.
*/

/*!
\qmlsignal void WebView::navigationHistoryChanged()
\deprecated
*/

/*!
\qmlsignal void WebView::loadingChanged(LoadEvent event)
\deprecated
*/

/*!
\fn void OxideQQuickWebView::loadingChanged(const OxideQLoadEvent& event)
\deprecated
*/

OxideQQuickWebView::OxideQQuickWebView(OxideQQuickWebViewPrivate& dd,
                                       QQuickItem* parent)
    : QQuickItem(parent),
      d_ptr(&dd) {
  Q_D(OxideQQuickWebView);

  d->contents_view_.reset(new oxide::qquick::ContentsView(this));
  if (d->aux_ui_factory_) {
    d->aux_ui_factory_->set_item(this);
  }

  oxide::qquick::EnsureChromiumStarted();

  setFlags(QQuickItem::ItemClipsChildrenToShape |
           QQuickItem::ItemHasContents |
           QQuickItem::ItemIsFocusScope |
           QQuickItem::ItemAcceptsDrops);
  setAcceptedMouseButtons(Qt::AllButtons);
  setAcceptHoverEvents(true);

  connect(&d->navigation_history_, &OxideQQuickNavigationHistory::changed,
          this, &OxideQQuickWebView::navigationHistoryChanged);
}

void OxideQQuickWebView::connectNotify(const QMetaMethod& signal) {
  Q_D(OxideQQuickWebView);

  Q_ASSERT(thread() == QThread::currentThread());

#define VIEW_SIGNAL(sig) QMetaMethod::fromSignal(&OxideQQuickWebView::sig)
  if (signal == VIEW_SIGNAL(newViewRequested) && d->proxy_) {
    d->proxy_->syncWebPreferences();
  } else if (signal == VIEW_SIGNAL(loadingChanged)) {
    WARN_DEPRECATED_API_USAGE() <<
        "OxideQQuickWebView: loadingChanged is deprecated. Please connect "
        "loadEvent if you want load event notifications, or "
        "loadingStateChanged if you want to detect changes to the loading "
        "property";
    d->using_old_load_event_signal_ = true;
  }
#undef VIEW_SIGNAL
}

void OxideQQuickWebView::disconnectNotify(const QMetaMethod& signal) {
  Q_D(OxideQQuickWebView);

  Q_ASSERT(thread() == QThread::currentThread());

  if ((signal == QMetaMethod::fromSignal(
          &OxideQQuickWebView::newViewRequested) ||
      !signal.isValid()) && d->proxy_) {
    d->proxy_->syncWebPreferences();
  }
}

bool OxideQQuickWebView::event(QEvent* event) {
  if (event->type() == GetPrepareToCloseBypassEventType()) {
    emit prepareToCloseResponse(true);
    return true;
  }

  return QQuickItem::event(event);
}

void OxideQQuickWebView::componentComplete() {
  Q_D(OxideQQuickWebView);

  Q_ASSERT(!d->constructed_);
  d->constructed_ = true;

  QQuickItem::componentComplete();

  OxideQQuickWebContext* context = context = d->construct_props_->context;

  if (!context && !d->construct_props_->new_view_request) {
    context = OxideQQuickWebContext::defaultContext(true);
    if (!context) {
      qFatal("OxideQQuickWebView: No context available!");
    }
    OxideQQuickWebContextPrivate* cd =
        OxideQQuickWebContextPrivate::get(context);
    d->construct_props_->context = context;
    d->attachContextSignals(cd);
  }

  if (d->construct_props_->new_view_request ||
      OxideQQuickWebContextPrivate::get(context)->isConstructed()) {
    d->completeConstruction();
  }
}

QVariant OxideQQuickWebView::inputMethodQuery(
    Qt::InputMethodQuery query) const {
  Q_D(const OxideQQuickWebView);

  return d->contents_view_->inputMethodQuery(query);
}

void OxideQQuickWebView::itemChange(QQuickItem::ItemChange change,
                                    const QQuickItem::ItemChangeData& value) {
  Q_D(OxideQQuickWebView);

  QQuickItem::itemChange(change, value);
  d->contents_view_->handleItemChange(change);
}

void OxideQQuickWebView::keyPressEvent(QKeyEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::keyPressEvent(event);
  d->contents_view_->handleKeyPressEvent(event);
}

void OxideQQuickWebView::keyReleaseEvent(QKeyEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::keyReleaseEvent(event);
  d->contents_view_->handleKeyReleaseEvent(event);
}

void OxideQQuickWebView::inputMethodEvent(QInputMethodEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::inputMethodEvent(event);
  d->contents_view_->handleInputMethodEvent(event);
}

void OxideQQuickWebView::focusInEvent(QFocusEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::focusInEvent(event);
  d->contents_view_->handleFocusInEvent(event);
}

void OxideQQuickWebView::focusOutEvent(QFocusEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::focusOutEvent(event);
  d->contents_view_->handleFocusOutEvent(event);
}

void OxideQQuickWebView::mousePressEvent(QMouseEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::mousePressEvent(event);
  d->contents_view_->handleMousePressEvent(event);
}

void OxideQQuickWebView::mouseMoveEvent(QMouseEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::mouseMoveEvent(event);
  d->contents_view_->handleMouseMoveEvent(event);
}

void OxideQQuickWebView::mouseReleaseEvent(QMouseEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::mouseReleaseEvent(event);
  d->contents_view_->handleMouseReleaseEvent(event);
}

void OxideQQuickWebView::touchUngrabEvent() {
  Q_D(OxideQQuickWebView);

  QQuickItem::touchUngrabEvent();
  d->contents_view_->handleTouchUngrabEvent();
}

void OxideQQuickWebView::wheelEvent(QWheelEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::wheelEvent(event);
  d->contents_view_->handleWheelEvent(event);
}

void OxideQQuickWebView::touchEvent(QTouchEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::touchEvent(event);
  d->contents_view_->handleTouchEvent(event);
}

void OxideQQuickWebView::hoverEnterEvent(QHoverEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::hoverEnterEvent(event);
  d->contents_view_->handleHoverEnterEvent(event);
}

void OxideQQuickWebView::hoverMoveEvent(QHoverEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::hoverMoveEvent(event);
  d->contents_view_->handleHoverMoveEvent(event);
}

void OxideQQuickWebView::hoverLeaveEvent(QHoverEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::hoverLeaveEvent(event);
  d->contents_view_->handleHoverLeaveEvent(event);
}

void OxideQQuickWebView::dragEnterEvent(QDragEnterEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::dragEnterEvent(event);
  d->contents_view_->handleDragEnterEvent(event);
}

void OxideQQuickWebView::dragMoveEvent(QDragMoveEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::dragMoveEvent(event);
  d->contents_view_->handleDragMoveEvent(event);
}

void OxideQQuickWebView::dragLeaveEvent(QDragLeaveEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::dragLeaveEvent(event);
  d->contents_view_->handleDragLeaveEvent(event);
}

void OxideQQuickWebView::dropEvent(QDropEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::dropEvent(event);
  d->contents_view_->handleDropEvent(event);
}

void OxideQQuickWebView::geometryChanged(const QRectF& newGeometry,
                                         const QRectF& oldGeometry) {
  Q_D(OxideQQuickWebView);

  QQuickItem::geometryChanged(newGeometry, oldGeometry);
  d->contents_view_->handleGeometryChanged();
}

QSGNode* OxideQQuickWebView::updatePaintNode(
    QSGNode* oldNode,
    UpdatePaintNodeData * updatePaintNodeData) {
  Q_UNUSED(updatePaintNodeData);
  Q_D(OxideQQuickWebView);

  return d->contents_view_->updatePaintNode(oldNode);
}

/*!
\internal
*/

OxideQQuickWebView::OxideQQuickWebView(QQuickItem* parent)
    : OxideQQuickWebView(*new OxideQQuickWebViewPrivate(this, nullptr),
                         parent) {}

OxideQQuickWebView::~OxideQQuickWebView() {
  Q_D(OxideQQuickWebView);

  if (d->contextHandle()) {
    d->detachContextSignals(
        OxideQQuickWebContextPrivate::get(
          qobject_cast<OxideQQuickWebContext*>(d->contextHandle())));
  }

  // Do this before our d_ptr is cleared, as these call back in to us
  // when they are deleted
  while (d->messageHandlers().size() > 0) {
    delete d->messageHandlers().at(0);
  }

  d->proxy_->teardownFrameTree();
}

/*!
\qmlproperty url WebView::url

The visible URL. This is the URL that a web browser should display in its
addressbar.

This will indicate the pending URL if a navigation is in progress and the
navigation is application initiated, or it is renderer initiated and this
webview hasn't been accessed by another webview.

Writing a URL to this property will initiate a new navigation.

\note Applications should not base any policy or security decisions on the value
of this property.
*/

QUrl OxideQQuickWebView::url() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return QUrl();
  }

  return d->proxy_->url();
}

void OxideQQuickWebView::setUrl(const QUrl& url) {
  Q_D(OxideQQuickWebView);

  QUrl old_url = this->url();

  if (!d->proxy_) {
    d->construct_props_->load_html = false;
    d->construct_props_->url = url;
    d->construct_props_->html.clear();
  } else {
    d->proxy_->setUrl(url);
  }

  if (this->url() != old_url) {
    // XXX(chrisccoulson): Why is this here? Don't we get this via URLChanged?
    emit urlChanged();
  }
}

/*!
\qmlproperty string WebView::title

The title of the currently displayed page. This is based on the \e{<title>} tag
or \e{document.title}.
*/

QString OxideQQuickWebView::title() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return QString();
  }

  return d->proxy_->title();
}

/*!
\qmlproperty url WebView::icon

The URL of the favicon for the current page. This will correspond to the URL
specified by the \e{<link rel="icon">} element if it exists in the page, else it
will fallback to \e{<domain>/favicon.ico} if the page is loaded over http: or
https:.

The application is responsible for downloading the icon. The icon specified by
this property is not guaranteed to exist.

There is currently no support for other favicon types (\e{<link
rel="apple-touch-icon">} or \e{<link rel="apple-touch-icon-precomposed">}).
*/

QUrl OxideQQuickWebView::icon() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return QUrl();
  }

  return d->proxy_->favIconUrl();
}

/*!
\qmlproperty bool WebView::canGoBack
\deprecated

This property will be true if the application can navigate the webview back by
calling goBack. Else it will be false.

\note The notification signal for this is \l{navigationHistoryChanged}.

\sa goBack
*/

bool OxideQQuickWebView::canGoBack() const {
  Q_D(const OxideQQuickWebView);

  WARN_DEPRECATED_API_USAGE() <<
      "OxideQQuickWebView: canGoBack is deprecated. Please use the API "
      "provided by OxideQQuickNavigationHistory instead";
  return d->navigation_history_.canGoBack();
}

/*!
\qmlproperty bool WebView::canGoForward
\deprecated

This property will be true if the application can navigate the webview forward
by calling goForward. Else it will be false.

\note The notification signal for this is \l{navigationHistoryChanged}.

\sa goForward
*/

bool OxideQQuickWebView::canGoForward() const {
  Q_D(const OxideQQuickWebView);

  WARN_DEPRECATED_API_USAGE() <<
      "OxideQQuickWebView: canGoForward is deprecated. Please use the API "
      "provided by OxideQQuickNavigationHistory instead";
  return d->navigation_history_.canGoForward();
}

/*!
\qmlproperty bool WebView::incognito

Whether the webview is in incognito mode. This can be set by the application
during construction - attempts to change it afterwards will be ignored.

The default is false.

Incognito webviews won't save browsing activity such as network cache, cookies
and local storage. They also won't reveal currently saved items to web pages.

If this WebView is created as a request to open a new window (via
newViewRequested), then the incognito setting will be inherited from the opening
WebView. Setting this will have no effect.
*/

bool OxideQQuickWebView::incognito() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return d->construct_props_->incognito;
  }

  return d->proxy_->incognito();
}

/*!
\internal
*/

void OxideQQuickWebView::setIncognito(bool incognito) {
  Q_D(OxideQQuickWebView);

  if (d->proxy_) {
    qWarning() <<
        "OxideQQuickWebView: incognito can only be set during construction";
    return;
  }

  if (oxideGetProcessModel() == OxideProcessModelSingleProcess && incognito) {
    qWarning() <<
        "OxideQQuickWebView: Cannot set incognito in single-process mode";
    return;
  }

  if (incognito == d->construct_props_->incognito) {
    return;
  }

  d->construct_props_->incognito = incognito;
  emit incognitoChanged();
}

/*!
\qmlproperty bool WebView::loading

This property indicates whether a load is currently in progress.

\note The notifier signal for this property is \l{loadingStateChanged} rather
than the expected \e{loadingChanged}.
*/

bool OxideQQuickWebView::loading() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return false;
  }

  return d->proxy_->loading();
}

/*!
\qmlproperty bool WebView::fullscreen

Whether fullscreen has been granted to this webview.

Applications should set this to true in response to a request from web content
to enter fullscreen mode (via the fullscreenRequested signal) if it entered
fullscreen. Doing this will cause Oxide to indicate to web content that it is
now fullscreen, and will result in the fullscreen element occupying the whole
webview.

Applications should set this to false again in response to a request
from web content to leave fullscreen mode or if the application exits
fullscreen.

\note Oxide does not control the fullscreen mode of the application window. It
is the responsibility of the application to do this, and it is the
responsibility of the application to hide its UI when entering fullscreen.
*/

bool OxideQQuickWebView::fullscreen() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return d->construct_props_->fullscreen;
  }

  return d->proxy_->fullscreen();
}

void OxideQQuickWebView::setFullscreen(bool fullscreen) {
  Q_D(OxideQQuickWebView);

  if (fullscreen == this->fullscreen()) {
    return;
  }

  if (!d->proxy_) {
    d->construct_props_->fullscreen = fullscreen;
  } else {
    d->proxy_->setFullscreen(fullscreen);
  }

  emit fullscreenChanged();
}

/*!
\qmlproperty int WebView::loadProgress

The current load progress as a percentage. The value of this can range between 0
and 100.

When \l{loading} is false, this will be 100.
*/

int OxideQQuickWebView::loadProgress() const {
  Q_D(const OxideQQuickWebView);

  return d->load_progress_;
}

/*!
\qmlproperty WebFrame WebView::rootFrame

The WebFrame representing the root frame for this webview.
*/

OxideQQuickWebFrame* OxideQQuickWebView::rootFrame() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return nullptr;
  }

  QObject* frame = d->proxy_->rootFrame();
  if (!frame) {
    return nullptr;
  }

  return qobject_cast<OxideQQuickWebFrame*>(frame);
}

/*!
\qmlproperty list<ScriptMessageHandler> WebView::messageHandlers

The list of script message handlers attached to this webview. These can handle
messages from user scripts injected in to frames inside this webview that
aren't handled by a ScriptMessageHandler attached to the destination WebFrame.
*/

QQmlListProperty<OxideQQuickScriptMessageHandler>
OxideQQuickWebView::messageHandlers() {
  return QQmlListProperty<OxideQQuickScriptMessageHandler>(
      this, nullptr,
      OxideQQuickWebViewPrivate::messageHandler_append,
      OxideQQuickWebViewPrivate::messageHandler_count,
      OxideQQuickWebViewPrivate::messageHandler_at,
      OxideQQuickWebViewPrivate::messageHandler_clear);
}

/*!
\qmlmethod void WebView::addMessageHandler(ScriptMessageHandler handler)

Add \a{handler} to the list of script message handlers attached to this view.
If \a{handler} is already attached to another WebView or WebFrame, then this
will return without making any changes.

If \a{handler} is already attached to this view then it will be moved to the
end of the list.

This view will assume ownership of \a{handle} by setting itself as the
parent.

If \a{handle} is deleted whilst it is attached to this view, then it will
automatically be detached.
*/

void OxideQQuickWebView::addMessageHandler(
    OxideQQuickScriptMessageHandler* handler) {
  Q_D(OxideQQuickWebView);

  if (!handler) {
    qWarning() << "OxideQQuickWebView::addMessageHandler: NULL handler";
    return;
  }

  OxideQQuickScriptMessageHandlerPrivate* hd =
      OxideQQuickScriptMessageHandlerPrivate::get(handler);

  if (hd->isActive() && handler->parent() != this) {
    qWarning() <<
        "OxideQQuickWebView::addMessageHandler: handler can't be added to "
        "more than one message target";
    return;
  }

  if (d->messageHandlers().contains(handler)) {
    d->messageHandlers().removeOne(handler);
  }

  handler->setParent(this);
  d->messageHandlers().append(handler);

  emit messageHandlersChanged();
}

/*!
\qmlmethod void WebView::removeMessageHandler(ScriptMessageHandler handler)

Remove \a{handler} from the list of script message handlers attached to this
view.

This will unparent \a{handler}, giving ownership of it back to the application.
*/

void OxideQQuickWebView::removeMessageHandler(
    OxideQQuickScriptMessageHandler* handler) {
  Q_D(OxideQQuickWebView);

  if (!handler) {
    qWarning() << "OxideQQuickWebView::removeMessageHandler: NULL handler";
    return;
  }

  if (!d->messageHandlers().contains(handler)) {
    return;
  }

  handler->setParent(nullptr);
  d->messageHandlers().removeOne(handler);

  emit messageHandlersChanged();
}

/*!
\property OxideQQuickWebView::viewportWidth
\deprecated
*/

/*!
\qmlproperty real WebView::viewportWidth
\deprecated
*/

qreal OxideQQuickWebView::viewportWidth() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return 0.f;
  }

  return const_cast<OxideQQuickWebViewPrivate*>(
      d)->proxy_->compositorFrameViewportSize().width();
}

/*!
\property OxideQQuickWebView::viewportHeight
\deprecated
*/

/*!
\qmlproperty real WebView::viewportHeight
\deprecated
*/

qreal OxideQQuickWebView::viewportHeight() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return 0.f;
  }

  return const_cast<OxideQQuickWebViewPrivate*>(
      d)->proxy_->compositorFrameViewportSize().height();
}

/*!
\property OxideQQuickWebView::contentWidth
\deprecated
*/

/*!
\qmlproperty real WebView::contentWidth
\deprecated
*/

qreal OxideQQuickWebView::contentWidth() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return 0.f;
  }

  return const_cast<OxideQQuickWebViewPrivate*>(
      d)->proxy_->compositorFrameContentSize().width();
}

/*!
\property OxideQQuickWebView::contentHeight
\deprecated
*/

/*!
\qmlproperty real WebView::contentHeight
\deprecated
*/

qreal OxideQQuickWebView::contentHeight() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return 0.f;
  }

  return const_cast<OxideQQuickWebViewPrivate*>(
      d)->proxy_->compositorFrameContentSize().height();
}

/*!
\property OxideQQuickWebView::contentX
\deprecated
*/

/*!
\qmlproperty real WebView::contentX
\deprecated
*/

qreal OxideQQuickWebView::contentX() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return 0.f;
  }

  return const_cast<OxideQQuickWebViewPrivate*>(
      d)->proxy_->compositorFrameScrollOffset().x();
}

/*!
\property OxideQQuickWebView::contentY
\deprecated
*/

/*!
\qmlproperty real WebView::contentY
\deprecated
*/

qreal OxideQQuickWebView::contentY() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return 0.f;
  }

  return const_cast<OxideQQuickWebViewPrivate*>(
      d)->proxy_->compositorFrameScrollOffset().y();
}

/*!
\qmlproperty Component WebView::contextMenu
\since OxideQt 1.8
*/

QQmlComponent* OxideQQuickWebView::contextMenu() const {
  Q_D(const OxideQQuickWebView);

  return d->context_menu_;
}

void OxideQQuickWebView::setContextMenu(QQmlComponent* contextMenu) {
  Q_D(OxideQQuickWebView);

  if (d->context_menu_ == contextMenu) {
    return;
  }

  if (d->aux_ui_factory_) {
    qWarning() <<
        "OxideQQuickWebView: Specifying a contextMenu implementation has no "
        "effect on this WebView, as it provides a built-in context menu";
  }

  d->context_menu_ = contextMenu;
  emit contextMenuChanged();
}

/*!
\qmlproperty Component WebView::popupMenu
*/

QQmlComponent* OxideQQuickWebView::popupMenu() const {
  Q_D(const OxideQQuickWebView);

  return d->contents_view_->popupMenu();
}

void OxideQQuickWebView::setPopupMenu(QQmlComponent* popupMenu) {
  Q_D(OxideQQuickWebView);

  if (d->contents_view_->popupMenu() == popupMenu) {
    return;
  }

  d->contents_view_->setPopupMenu(popupMenu);
  emit popupMenuChanged();
}

/*!
\qmlproperty Component WebView::alertDialog
*/

QQmlComponent* OxideQQuickWebView::alertDialog() const {
  Q_D(const OxideQQuickWebView);

  return d->alert_dialog_;
}

void OxideQQuickWebView::setAlertDialog(QQmlComponent* alertDialog) {
  Q_D(OxideQQuickWebView);

  if (d->alert_dialog_ == alertDialog) {
    return;
  }

  d->alert_dialog_ = alertDialog;
  emit alertDialogChanged();
}

/*!
\qmlproperty Component WebView::confirmDialog
*/

QQmlComponent* OxideQQuickWebView::confirmDialog() const {
  Q_D(const OxideQQuickWebView);

  return d->confirm_dialog_;
}

void OxideQQuickWebView::setConfirmDialog(QQmlComponent* confirmDialog) {
  Q_D(OxideQQuickWebView);

  if (d->confirm_dialog_ == confirmDialog) {
    return;
  }

  d->confirm_dialog_ = confirmDialog;
  emit confirmDialogChanged();
}

/*!
\qmlproperty Component WebView::promptDialog
*/

QQmlComponent* OxideQQuickWebView::promptDialog() const {
  Q_D(const OxideQQuickWebView);

  return d->prompt_dialog_;
}

void OxideQQuickWebView::setPromptDialog(QQmlComponent* promptDialog) {
  Q_D(OxideQQuickWebView);

  if (d->prompt_dialog_ == promptDialog) {
    return;
  }

  d->prompt_dialog_ = promptDialog;
  emit promptDialogChanged();
}

/*!
\qmlproperty Component WebView::beforeUnloadDialog
*/

QQmlComponent* OxideQQuickWebView::beforeUnloadDialog() const {
  Q_D(const OxideQQuickWebView);

  return d->before_unload_dialog_;
}

void OxideQQuickWebView::setBeforeUnloadDialog(
    QQmlComponent* beforeUnloadDialog) {
  Q_D(OxideQQuickWebView);

  if (d->before_unload_dialog_ == beforeUnloadDialog) {
    return;
  }

  d->before_unload_dialog_ = beforeUnloadDialog;
  emit beforeUnloadDialogChanged();
}

/*!
\qmlproperty Component WebView::filePicker
*/

QQmlComponent* OxideQQuickWebView::filePicker() const {
  Q_D(const OxideQQuickWebView);

  return d->file_picker_;
}

void OxideQQuickWebView::setFilePicker(QQmlComponent* filePicker) {
  Q_D(OxideQQuickWebView);

  if (d->file_picker_ == filePicker) {
    return;
  }

  d->file_picker_ = filePicker;
  emit filePickerChanged();
}

/*!
\qmlproperty WebContext WebView::context

The WebContext to use for this WebView. This can be set during construction -
attempts to change it afterwards will be ignored.

If the application doesn't set this, then WebView will use the application
default WebContext (Oxide::defaultWebContext).

The application should ensure that the provided WebContext outlives this
WebView. Deleting the WebContext whilst the WebView is still alive may cause
some features to stop working.

If this WebView is created as a request to open a new window (via
newViewRequested), then the WebContext will be inherited from the opening
WebView. Setting this will have no effect.

Attempts to set this when Oxide::processModel is
\e{Oxide.ProcessModelSingleProcess} will be ignored. Only the default WebContext
can be used in single process mode.
*/

OxideQQuickWebContext* OxideQQuickWebView::context() const {
  Q_D(const OxideQQuickWebView);

  QObject* c = d->contextHandle();
  if (!c) {
    return nullptr;
  }

  return qobject_cast<OxideQQuickWebContext*>(c);
}

/*!
\internal
*/

void OxideQQuickWebView::setContext(OxideQQuickWebContext* context) {
  Q_D(OxideQQuickWebView);

  if (d->proxy_) {
    qWarning() <<
        "OxideQQuickWebView: context can only be set during construction";
    return;
  }

  if (oxideGetProcessModel() == OxideProcessModelSingleProcess) {
    qWarning() <<
        "OxideQQuickWebView: context is read-only in single process mode. "
        "The webview will automatically use the application-wide default "
        "WebContext";
    return;
  }

  OxideQQuickWebContext* old = this->context();

  if (context == old) {
    return;
  }

  if (old) {
    d->detachContextSignals(OxideQQuickWebContextPrivate::get(old));
  }

  if (context) {
    d->attachContextSignals(OxideQQuickWebContextPrivate::get(context));
  }
  d->construct_props_->context = context;

  emit contextChanged();
}

/*!
\qmlproperty WebPreferences WebView::preferences

The WebPreferences object used to customize the behaviour of this WebView.
Please refer to the documentation for WebPreferences for more information about
how to use this.

This always returns a valid WebPreferences instance.

The default WebPreferences instance is a child of this WebView.

Applications can set this to a different WebPreferences instance. If the
provided instance does not have a parent, then this WebView will set itself as
the parent. When setting a new WebPreferences instance, the old instance will
be deleted if this WebView is its parent.

If this WebView is created as a request to open a new window (via
newViewRequested), then the default WebPreferences will be copied from the one
provided by the opening WebView.

If this view's WebPreferences instance is deleted, a default WebPreferences
instance will be recreated automatically for this WebView.
*/

OxideQWebPreferences* OxideQQuickWebView::preferences() {
  Q_D(OxideQQuickWebView);

  if (!d->proxy_) {
    if (!d->construct_props_->preferences) {
      d->construct_props_->preferences = new OxideQWebPreferences(this);
      d->attachPreferencesSignals(d->construct_props_->preferences);
    }
    return d->construct_props_->preferences;
  }

  OxideQWebPreferences* prefs = d->proxy_->preferences();
  Q_ASSERT(prefs);

  return prefs;
}

void OxideQQuickWebView::setPreferences(OxideQWebPreferences* prefs) {
  Q_D(OxideQQuickWebView);

  OxideQWebPreferences* old = preferences();
  if (prefs == old && prefs) {
    return;
  }

  if (!prefs) {
    prefs = new OxideQWebPreferences(this);
  }

  if (!prefs->parent()) {
    prefs->setParent(this);
  }
  d->attachPreferencesSignals(prefs);

  if (!d->proxy_) {
    d->construct_props_->preferences = prefs;
  } else {
    d->proxy_->setPreferences(prefs);
  }

  if (old) {
    old->disconnect(this, SLOT(preferencesDestroyed()));
    if (old->parent() == this) {
      delete old;
    }
  }

  emit preferencesChanged();
}

/*!
\qmlproperty NavigationHistory WebView::navigationHistory

The navigation history for this webview. Please refer to the documentation for
NavigationHistory for more information about how to use this.
*/

OxideQQuickNavigationHistory* OxideQQuickWebView::navigationHistory() {
  Q_D(OxideQQuickWebView);

  return &d->navigation_history_;
}

/*!
\qmlproperty SecurityStatus WebView::securityStatus
\since OxideQt 1.2

The security status for this WebView. Please refer to the documentation for
SecurityStatus for more information about how to use this.
*/

OxideQSecurityStatus* OxideQQuickWebView::securityStatus() {
  Q_D(OxideQQuickWebView);

  return d->security_status_.data();
}

/*!
\qmlproperty flags WebView::blockedContent
\since OxideQt 1.2

This indicates content that has been blocked in this WebView.

Possible values are:

\value WebView.ContentTypeNone
(0) No content is blocked

\value WebView.ContentTypeMixedDisplay
Passive mixed content was blocked on this page. This will only happen if
WebPreferences::canDisplayInsecureContent is set to false. The application can
temporarily override this by calling setCanTemporarilyDisplayInsecureContent.

\value WebView.ContentTypeMixedScript
Active mixed content was blocked on this page. This will only happen if
WebPreferences::canRunInsecureContent is set to false. The application can
temporarily override this by calling setCanTemporarilyRunInsecureContent.
*/

OxideQQuickWebView::ContentType OxideQQuickWebView::blockedContent() const {
  Q_D(const OxideQQuickWebView);

  Q_STATIC_ASSERT(
      ContentTypeNone ==
        static_cast<ContentTypeFlags>(oxide::qt::CONTENT_TYPE_NONE));
  Q_STATIC_ASSERT(
      ContentTypeMixedDisplay ==
        static_cast<ContentTypeFlags>(oxide::qt::CONTENT_TYPE_MIXED_DISPLAY));
  Q_STATIC_ASSERT(
      ContentTypeMixedScript ==
        static_cast<ContentTypeFlags>(oxide::qt::CONTENT_TYPE_MIXED_SCRIPT));

  if (!d->proxy_) {
    return ContentTypeNone;
  }

  return static_cast<ContentType>(d->proxy_->blockedContent());
}

/*!
\property OxideQQuickWebView::request
\internal
*/

/*!
\qmlproperty NewViewRequest WebView::request

This should be set during construction of a WebView created in response to
newViewRequested. Please refer to the documentation for newViewRequested.

This is a write-only property. Reads from it always return null.
*/

/*!
\internal
*/

// This exists purely to remove a moc warning. We don't store this request
// anywhere, it's only a transient object and I can't think of any possible
// reason why anybody would want to read it back
OxideQNewViewRequest* OxideQQuickWebView::request() const {
  return nullptr;
}

/*!
\internal
*/

void OxideQQuickWebView::setRequest(OxideQNewViewRequest* request) {
  Q_D(OxideQQuickWebView);

  if (d->proxy_) {
    qWarning() <<
        "OxideQQuickWebView: request must be provided during construction";
    return;
  }

  d->construct_props_->new_view_request = request;
}

/*!
\property OxideQQuickWebView::restoreState
\internal
*/

/*!
\qmlproperty string WebView::restoreState
\since OxideQt 1.4

This should be set during construction of a WebView to restore the state from a
previous session. To do this, the application can set this to the serialized
state obtained from currentState in a previous session.

If this WebView is created as a request to open a new window (via
newViewRequested), then setting this has no effect.

This is a write-only property. Reads from it always return an empty string.
*/

/*!
\internal
*/

// This exists purely to remove a moc warning. We don't store this initial state
// anywhere, it's only a transient blob and I can't think of any possible
// reason why anybody would want to read it back
QString OxideQQuickWebView::restoreState() const {
  return QString();
}

/*!
\internal
*/

void OxideQQuickWebView::setRestoreState(const QString& state) {
  Q_D(OxideQQuickWebView);

  if (d->proxy_) {
    qWarning() <<
        "OxideQQuickWebView: restoreState must be provided during construction";
    return;
  }

  // state is expected to be a base64-encoded string
  d->construct_props_->restore_state =
      QByteArray::fromBase64(state.toLocal8Bit());
}

/*!
\property OxideQQuickWebView::restoreType
\internal
*/

/*!
\qmlproperty enumeration WebView::restoreType
\since OxideQt 1.4

This should be set during construction of a WebView in conjunction with
restoreState. It defines the type of restore, which may affect things
like cache policy.

Possible values are:

\value WebView.RestoreCurrentSession
This WebView is being restored from a WebView that was closed in this session.

\value WebView.RestoreLastSessionExitedCleanly
This WebView is being restored from a WebView closed at the end of the previous
session.

\value WebView.RestoreLastSessionCrashed
This WebView is being restored from a WebView 

This is a write-only property. Reads from it always return
\e{RestoreLastSessionExitedCleanly}.
*/

/*!
\internal
*/

// This exists purely to remove a moc warning. We don't store this restore type
// anywhere, it's only a transient property and I can't think of any possible
// reason why anybody would want to read it back
OxideQQuickWebView::RestoreType OxideQQuickWebView::restoreType() const {
  Q_D(const OxideQQuickWebView);

  return RestoreLastSessionExitedCleanly;
}

/*!
\internal
*/

void OxideQQuickWebView::setRestoreType(OxideQQuickWebView::RestoreType type) {
  Q_D(OxideQQuickWebView);

  if (d->proxy_) {
    qWarning() <<
        "OxideQQuickWebView: restoreType must be provided during construction";
    return;
  }

  d->construct_props_->restore_type = ToInternalRestoreType(type);
}

/*!
\qmlproperty string WebView::currentState
\since OxideQt 1.4

This is the serialized state of the WebView. If this WebView is deleted, it
will be possible to restore it by passing the serialized state to restoreState
when constructing a new WebView.

This property is currently not notifiable.
*/

QString OxideQQuickWebView::currentState() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return QString();
  }

  // Encode the current state in base64 so it can be safely passed around
  // as a string (QML doesn’t know of byte arrays)
  return QString::fromLocal8Bit(d->proxy_->currentState().toBase64());
}

/*!
\qmlproperty LocationBarController WebView::locationBarController
\since OxideQt 1.5

This is the LocationBarController instance for this WebView. Please refer to the
documentation for LocationBarController for more information about how to use
it.
*/

OxideQQuickLocationBarController* OxideQQuickWebView::locationBarController() {
  Q_D(OxideQQuickWebView);

  if (!d->location_bar_controller_) {
    d->location_bar_controller_ =
        OxideQQuickLocationBarControllerPrivate::create();
    if (d->proxy_) {
      OxideQQuickLocationBarControllerPrivate::get(
          d->location_bar_controller_.get())->init(d->proxy_->webContentsID());
    }
  }

  return d->location_bar_controller_.get();
}

/*!
\qmlproperty enumeration WebView::webProcessStatus
\since OxideQt 1.8

This is the current state of the process used for running web content in this
WebView.

Possible values are:

\value WebView.WebProcessRunning
The web content process is running normally. This value is also used if this
WebView doesn't yet have a web content process.

\value WebView.WebProcessKilled
The web content process has been killed by the system or a signal sent to it.
This might happen as a result of low memory or application lifecycle management.

\value WebView.WebProcessCrashed
The web content process has crashed or otherwise exited abnormally.

\value WebView.WebProcessUnresponsive
The web content process is no longer responding to events. The timeout for
triggering this is currently set to 5s.

If Oxide::processModel is \e{Oxide.ProcessModelSingleProcess}, this will only
ever be \e{WebProcessRunning} or \e{WebProcessUnresponsive}
*/

OxideQQuickWebView::WebProcessStatus OxideQQuickWebView::webProcessStatus() const {
  Q_D(const OxideQQuickWebView);

  STATIC_ASSERT_MATCHING_ENUM(WebProcessRunning,
                              oxide::qt::WEB_PROCESS_RUNNING);
  STATIC_ASSERT_MATCHING_ENUM(WebProcessKilled,
                              oxide::qt::WEB_PROCESS_KILLED);
  STATIC_ASSERT_MATCHING_ENUM(WebProcessCrashed,
                              oxide::qt::WEB_PROCESS_CRASHED);
  STATIC_ASSERT_MATCHING_ENUM(WebProcessUnresponsive,
                              oxide::qt::WEB_PROCESS_UNRESPONSIVE);
  Q_STATIC_ASSERT(
      WebProcessRunning ==
        static_cast<WebProcessStatus>(oxide::qt::WEB_PROCESS_RUNNING));
  Q_STATIC_ASSERT(
      WebProcessKilled ==
        static_cast<WebProcessStatus>(oxide::qt::WEB_PROCESS_KILLED));
  Q_STATIC_ASSERT(
      WebProcessCrashed ==
        static_cast<WebProcessStatus>(oxide::qt::WEB_PROCESS_CRASHED));

  if (!d->proxy_) {
    return WebProcessRunning;
  }

  return static_cast<WebProcessStatus>(d->proxy_->webProcessStatus());
}

/*!
\qmlproperty url WebView::hoveredUrl
\since OxideQt 1.12

When using a pointing device, this indicates the URL of the link currently under
the pointer. If no link is under the pointer, then this will be null.

This will also indicate the URL of a link focused by the keyboard.
*/

QUrl OxideQQuickWebView::hoveredUrl() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return QUrl();
  }

  return d->proxy_->targetUrl();
}

/*!
\qmlproperty flags WebView::editingCapabilities
\since OxideQt 1.12

This indicates the editing commands that are currently active and available via
the executeEditingCommand function for this WebView.

Possible values are:

\value WebView.NoCapability
(0) No editing commands are available

\value WebView.UndoCapability
The \e{EditingCommandUndo} command is active

\value WebView.RedoCapability
The \e{EditingCommandRedo} command is active

\value WebView.CutCapability
The \e{EditingCommandCut} command is active

\value WebView.CopyCapability
The \e{EditingCommandCopy} command is active

\value WebView.PasteCapability
The \e{EditingCommandPaste} command is active

\value WebView.EraseCapability
The \e{EditingCommandErase} command is active

\value WebView.SelectAllCapability
The \e{EditingCommandSelectAll} command is active

\sa executeEditingCommand
*/

OxideQQuickWebView::EditCapabilities
OxideQQuickWebView::editingCapabilities() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return NoCapability;
  }

  oxide::qt::EditCapabilityFlags flags = d->proxy_->editFlags();
  return static_cast<EditCapabilities>(
      oxide::qt::EditCapabilityFlags::Int(flags));
}

/*!
\qmlproperty real WebView::zoomFactor
\since OxideQt 1.15

The page-zoom factor for this WebView. This is 1 by default.

Changes to this will also propagate to other webviews displaying a page from the
same host. The zoom factor will persist for the remainder of the session.

When the WebView is navigated to a new host, the zoom factor is either restored
to the default or restored to the last defined zoom factor for the new host.

Attempts to set the zoom factor to a value that is less than minimumZoomFactor
or greater than maximumZoomFactor will be ignored.

\sa minimumZoomFactor, maximumZoomFactor
*/

qreal OxideQQuickWebView::zoomFactor() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return 1.0;
  }

  return d->proxy_->zoomFactor();
}

void OxideQQuickWebView::setZoomFactor(qreal factor) {
  Q_D(OxideQQuickWebView);

  if (qFuzzyCompare(factor, zoomFactor())) {
    return;
  }

  if (!d->proxy_) {
    qWarning() <<
        "OxideQQuickWebView: zoom factor cannot be set during construction, "
        "it is a per-host value";
    return;
  }

  if (factor < minimumZoomFactor() || factor > maximumZoomFactor()) {
    qWarning() <<
        "OxideQQuickWebView: invalid value for zoom factor, expected to be "
        "between" << minimumZoomFactor() << "and" << maximumZoomFactor();
    return;
  }

  d->proxy_->setZoomFactor(factor);
}

/*!
\qmlproperty real WebView::minimumZoomFactor
\since OxideQt 1.15

The minimum permitted zoom factor. Attempts to set zoomFactor to a value less
than this will be ignored.

\sa zoomFactor
*/

qreal OxideQQuickWebView::minimumZoomFactor() const {
  return oxide::qt::WebViewProxy::minimumZoomFactor();
}

/*!
\qmlproperty real WebView::maximumZoomFactor
\since OxideQt 1.15

The maximum permitted zoom factor. Attempts to set zoomFactor to a value greater
than this will be ignored.

\sa zoomFactor
*/

qreal OxideQQuickWebView::maximumZoomFactor() const {
  return oxide::qt::WebViewProxy::maximumZoomFactor();
}

// static
OxideQQuickWebViewAttached* OxideQQuickWebView::qmlAttachedProperties(
    QObject* object) {
  return new OxideQQuickWebViewAttached(object);
}

/*!
\qmlmethod void WebView::executeEditingCommand(enumeration command)
\since OxideQt 1.8

Execute the editing command defined by \a{command}. If the specified command is
not defined as currently active by editingCapabilities, this will have no
effect.

Possible values for \a{command} are:

\value WebView.EditingCommandUndo
Undo the previous change

\value WebView.EditingCommandRedo
Redo the last undone change

\value WebView.EditingCommandCut
Delete the text that is currently selected, copying it to the clipboard in the
process

\value WebView.EditingCommandCopy
Copy the currently selected text to the clipboard

\value WebView.EditingCommandPaste
Paste the contents of the clipboard at the current cursor position

\value WebView.EditingCommandErase
Delete the text that is currently selected

\value WebView.EditingCommandSelectAll
Select all of the text in the editor
*/

void OxideQQuickWebView::executeEditingCommand(EditingCommands command) const {
  Q_D(const OxideQQuickWebView);

  Q_STATIC_ASSERT(
      EditingCommandUndo ==
        static_cast<EditingCommands>(oxide::qt::EDITING_COMMAND_UNDO));
  Q_STATIC_ASSERT(
      EditingCommandRedo ==
        static_cast<EditingCommands>(oxide::qt::EDITING_COMMAND_REDO));
  Q_STATIC_ASSERT(
      EditingCommandCut ==
        static_cast<EditingCommands>(oxide::qt::EDITING_COMMAND_CUT));
  Q_STATIC_ASSERT(
      EditingCommandCopy ==
        static_cast<EditingCommands>(oxide::qt::EDITING_COMMAND_COPY));
  Q_STATIC_ASSERT(
      EditingCommandPaste ==
        static_cast<EditingCommands>(oxide::qt::EDITING_COMMAND_PASTE));
  Q_STATIC_ASSERT(
      EditingCommandErase ==
        static_cast<EditingCommands>(oxide::qt::EDITING_COMMAND_ERASE));
  Q_STATIC_ASSERT(
      EditingCommandSelectAll ==
        static_cast<EditingCommands>(oxide::qt::EDITING_COMMAND_SELECT_ALL));

  if (!d->proxy_) {
    return;
  }

  d->proxy_->executeEditingCommand(
      static_cast<oxide::qt::EditingCommands>(command));
}

/*!
\qmlmethod void WebView::goBack()
\deprecated

Navigate to the previous entry in the navigation history. If there isn't one,
then calling this function will do nothing.

\sa canGoBack
*/

void OxideQQuickWebView::goBack() {
  Q_D(OxideQQuickWebView);

  WARN_DEPRECATED_API_USAGE() <<
      "OxideQQuickWebView: goBack is deprecated. Please use the API "
      "provided by OxideQQuickNavigationHistory instead";
  d->navigation_history_.goBack();
}

/*!
\qmlmethod void WebView::goForward()
\deprecated

Navigate to the next entry in the navigation history. If there isn't one, then
calling this function will do nothing.

\sa canGoForward
*/

void OxideQQuickWebView::goForward() {
  Q_D(OxideQQuickWebView);

  WARN_DEPRECATED_API_USAGE() <<
      "OxideQQuickWebView: goForward is deprecated. Please use the API "
      "provided by OxideQQuickNavigationHistory instead";
  d->navigation_history_.goForward();
}

/*!
\qmlmethod void WebView::stop()

Cancel the current navigation, if one is in progress. If there isn't currently a
navigation in progress, this will do nothing.

This will result in loadEvent being fired with a \e{LoadEvent.TypeStopped}
event.
*/

void OxideQQuickWebView::stop() {
  Q_D(OxideQQuickWebView);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->stop();
}

/*!
\qmlmethod void WebView::reload()

Reload the page that is currently displayed in the WebView.
*/

void OxideQQuickWebView::reload() {
  Q_D(OxideQQuickWebView);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->reload();
}

/*!
\qmlmethod void WebView::loadHtml(string html, url baseUrl = undefined)

Load the HTML specified by \a{html} in to this WebView. It will be loaded as a
percent encoded data: URL with the \e{mediatype} set to
"text/html;charset=UTF-8".

If \a{baseUrl} is specified, the provided URL will be used as the security
origin for the page.
*/

void OxideQQuickWebView::loadHtml(const QString& html, const QUrl& baseUrl) {
  Q_D(OxideQQuickWebView);

  if (!d->proxy_) {
    d->construct_props_->load_html = true;
    d->construct_props_->html = html;
    d->construct_props_->url = baseUrl;
    return;
  }

  d->proxy_->loadHtml(html, baseUrl);
}

/*!
\qmlmethod void WebView::setCanTemporarilyDisplayInsecureContent(bool allow)
\since OxideQt 1.2

Calling this allows the WebView to temporarily display passive mixed content.
This will only have an effect if blockedContent indicates that passive mixed
content is currently being blocked.

The effect of this call is temporary - passive mixed content will be disallowed
the next time a navigation is committed inside this WebView.

Calling this triggers a reload of the page.
*/

void OxideQQuickWebView::setCanTemporarilyDisplayInsecureContent(bool allow) {
  Q_D(OxideQQuickWebView);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->setCanTemporarilyDisplayInsecureContent(allow);
}

/*!
\qmlmethod void WebView::setCanTemporarilyRunInsecureContent(bool allow)
\since OxideQt 1.2

Calling this allows the WebView to temporarily run active mixed content. This
will only have an effect if blockedContent indicates that active mixed
content is currently being blocked.

The effect of this call is temporary - active mixed content will be disallowed
the next time a navigation is committed inside this WebView.

Calling this triggers a reload of the page.
*/

void OxideQQuickWebView::setCanTemporarilyRunInsecureContent(bool allow) {
  Q_D(OxideQQuickWebView);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->setCanTemporarilyRunInsecureContent(allow);
}

/*!
\qmlmethod void WebView::prepareToClose()
\since OxideQt 1.4

Causes Oxide to run the current page's \e{beforeunload} handler if there is one.
This should be called by applications before the WebView is deleted.

The response to this will be delivered asynchronously by the
prepareToCloseResponse signal.
*/

void OxideQQuickWebView::prepareToClose() {
  Q_D(OxideQQuickWebView);

  if (!d->proxy_) {
    QCoreApplication::postEvent(this,
                                new QEvent(GetPrepareToCloseBypassEventType()));
    return;
  }

  d->proxy_->prepareToClose();
}

/*!
\qmlmethod void WebView::terminateWebProcess()
\since OxideQt 1.19

Terminate the current web process. This call is asynchronous and won't wait for
the process to die before returning.

This function is useful for terminating web content processes that have become
unresponsive.

Calling this when Oxide::processModel is \e{Oxide.ProcessModelSingleProcess}
will result in the calling process being killed.
*/

void OxideQQuickWebView::terminateWebProcess() {
  Q_D(OxideQQuickWebView);

  if (!d->proxy_) {
    qWarning() <<
        "OxideQQuickWebView::terminateWebProcess: There is no web process yet";
    return;
  }

  d->proxy_->terminateWebProcess();
}

/*!
\qmlproperty FindController WebView::findController
\since OxideQt 1.8

The WebView's FindController instance. Please refer to the documentation for
FindController for more details about how to use it.
*/

OxideQFindController* OxideQQuickWebView::findController() const {
  Q_D(const OxideQQuickWebView);

  return d->find_controller_.data();
}

/*!
\qmlproperty TouchSelectionController WebView::touchSelectionController
\since OxideQt 1.12

The WebView's TouchSelectionController instance. Please refer to the
documentation for TouchSelectionController for more details about how to use it.
*/

OxideQQuickTouchSelectionController* OxideQQuickWebView::touchSelectionController() {
  Q_D(OxideQQuickWebView);

  return d->contents_view_->touchSelectionController();
}

#include "moc_oxideqquickwebview.cpp"
