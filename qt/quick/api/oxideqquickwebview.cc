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
#include <QGuiApplication>
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
#include <QScreen>
#include <QSGNode>
#include <QSizeF>
#include <QSize>
#include <QTouchEvent>
#include <QtQml>
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
#include "qt/quick/oxide_qquick_accelerated_frame_node.h"
#include "qt/quick/oxide_qquick_alert_dialog.h"
#include "qt/quick/oxide_qquick_before_unload_dialog.h"
#include "qt/quick/oxide_qquick_confirm_dialog.h"
#include "qt/quick/oxide_qquick_contents_view.h"
#include "qt/quick/oxide_qquick_file_picker.h"
#include "qt/quick/oxide_qquick_image_frame_node.h"
#include "qt/quick/oxide_qquick_init.h"
#include "qt/quick/oxide_qquick_prompt_dialog.h"
#include "qt/quick/oxide_qquick_software_frame_node.h"
#include "qt/quick/oxide_qquick_touch_handle_drawable.h"

#include "oxideqquicklocationbarcontroller.h"
#include "oxideqquickscriptmessagehandler.h"
#include "oxideqquickscriptmessagehandler_p.h"
#include "oxideqquicktouchselectioncontroller.h"
#include "oxideqquickwebcontext.h"
#include "oxideqquickwebcontext_p.h"
#include "oxideqquickwebframe.h"
#include "oxideqquickwebframe_p.h"


using oxide::qquick::AcceleratedFrameNode;
using oxide::qquick::ImageFrameNode;
using oxide::qquick::SoftwareFrameNode;

namespace {

QEvent::Type GetPrepareToCloseBypassEventType() {
  static int g_event_type = QEvent::registerEventType();
  return QEvent::Type(g_event_type);
}

}

class UpdatePaintNodeScope {
 public:
  UpdatePaintNodeScope(OxideQQuickWebViewPrivate* d)
      : d_(d) {}

  virtual ~UpdatePaintNodeScope() {
    d_->didUpdatePaintNode();
  }

 private:
  OxideQQuickWebViewPrivate* d_;
};

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
        location_bar_height(0),
        location_bar_mode(oxide::qt::LOCATION_BAR_MODE_AUTO),
        location_bar_animated(true),
        load_html(false),
        fullscreen(false) {}

  bool incognito;
  QPointer<OxideQQuickWebContext> context;
  QPointer<OxideQNewViewRequest> new_view_request;
  QByteArray restore_state;
  oxide::qt::RestoreType restore_type;
  QList<QObject*> message_handlers;
  int location_bar_height;
  oxide::qt::LocationBarMode location_bar_mode;
  bool location_bar_animated;
  bool load_html;
  QUrl url;
  QString html;
  bool fullscreen;
  QPointer<OxideQWebPreferences> preferences;
};

OxideQQuickWebViewPrivate::OxideQQuickWebViewPrivate(OxideQQuickWebView* view)
    : q_ptr(view),
      contents_view_(new oxide::qquick::ContentsView(view)),
      load_progress_(0),
      security_status_(OxideQSecurityStatusPrivate::Create()),
      find_controller_(OxideQFindControllerPrivate::Create()),
      constructed_(false),
      navigation_history_(view),
      alert_dialog_(nullptr),
      confirm_dialog_(nullptr),
      prompt_dialog_(nullptr),
      before_unload_dialog_(nullptr),
      file_picker_(nullptr),
      received_new_compositor_frame_(false),
      frame_evicted_(false),
      last_composited_frame_type_(oxide::qt::CompositorFrameHandle::TYPE_INVALID),
      using_old_load_event_signal_(false),
      handling_unhandled_key_event_(false),
      construct_props_(new ConstructProps()) {}

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

oxide::qt::TouchHandleDrawableProxy*
OxideQQuickWebViewPrivate::CreateTouchHandleDrawable() {
  Q_Q(OxideQQuickWebView);

  return new oxide::qquick::TouchHandleDrawable(q);
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

void OxideQQuickWebViewPrivate::CommandsUpdated() {
  Q_Q(OxideQQuickWebView);

  emit q->navigationHistoryChanged();
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

void OxideQQuickWebViewPrivate::NavigationEntryCommitted() {
  navigation_history_.onNavigationEntryCommitted();
}

void OxideQQuickWebViewPrivate::NavigationListPruned(bool from_front, int count) {
  navigation_history_.onNavigationListPruned(from_front, count);
}

void OxideQQuickWebViewPrivate::NavigationEntryChanged(int index) {
  navigation_history_.onNavigationEntryChanged(index);
}

void OxideQQuickWebViewPrivate::TouchSelectionChanged(bool active,
                                                      QRectF bounds) {
  Q_Q(OxideQQuickWebView);

  q->touchSelectionController()->onTouchSelectionChanged(active, bounds);
}

void OxideQQuickWebViewPrivate::CreateWebFrame(
    oxide::qt::WebFrameProxy* proxy) {
  Q_Q(OxideQQuickWebView);

  OxideQQuickWebFrame* frame = OxideQQuickWebFramePrivate::create(proxy);
  QQmlEngine::setObjectOwnership(frame, QQmlEngine::CppOwnership);

  emit q->frameAdded(frame);
}

bool OxideQQuickWebViewPrivate::IsVisible() const {
  Q_Q(const OxideQQuickWebView);

  return q->isVisible();
}

bool OxideQQuickWebViewPrivate::HasFocus() const {
  Q_Q(const OxideQQuickWebView);

  return q->hasActiveFocus() && (q->window() ? q->window()->isActive() : false);
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

void OxideQQuickWebViewPrivate::WebPreferencesReplaced() {
  Q_Q(OxideQQuickWebView);

  emit q->preferencesChanged();
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

void OxideQQuickWebViewPrivate::UpdateCursor(const QCursor& cursor) {
  Q_Q(OxideQQuickWebView);

  q->setCursor(cursor);
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

void OxideQQuickWebViewPrivate::HandleUnhandledKeyboardEvent(
    QKeyEvent* event) {
  Q_Q(OxideQQuickWebView);

  QQuickWindow* w = q->window();
  if (!w) {
    return;
  }

  Q_ASSERT(!handling_unhandled_key_event_);

  handling_unhandled_key_event_ = true;
  w->sendEvent(q, event);
  handling_unhandled_key_event_ = false;
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

  if (!location_bar_controller_) {
    return;
  }

  if (f.testFlag(oxide::qt::FRAME_METADATA_CHANGE_CONTROLS_OFFSET)) {
    emit location_bar_controller_->offsetChanged();
  }
  if (f.testFlag(oxide::qt::FRAME_METADATA_CHANGE_CONTENT_OFFSET)) {
    emit location_bar_controller_->contentOffsetChanged();
  }
}

void OxideQQuickWebViewPrivate::ScheduleUpdate() {
  Q_Q(OxideQQuickWebView);

  frame_evicted_ = false;
  received_new_compositor_frame_ = true;

  q->update();
}

void OxideQQuickWebViewPrivate::EvictCurrentFrame() {
  Q_Q(OxideQQuickWebView);

  frame_evicted_ = true;
  received_new_compositor_frame_ = false;

  q->update();
}

void OxideQQuickWebViewPrivate::SetInputMethodEnabled(bool enabled) {
  Q_Q(OxideQQuickWebView);

  q->setFlag(QQuickItem::ItemAcceptsInputMethod, enabled);
  QGuiApplication::inputMethod()->update(Qt::ImEnabled);
}

void OxideQQuickWebViewPrivate::DownloadRequested(
    const OxideQDownloadRequest& download_request) {
  Q_Q(OxideQQuickWebView);

  emit q->downloadRequested(download_request);
}

void OxideQQuickWebViewPrivate::CertificateError(
    OxideQCertificateError* cert_error) {
  Q_Q(OxideQQuickWebView);

  // See the comment in RequestGeolocationPermission
  QQmlEngine* engine = qmlEngine(q);
  if (!engine) {
    delete cert_error;
    return;
  }

  {
    QJSValue val = engine->newQObject(cert_error);
    if (!val.isQObject()) {
      delete cert_error;
      return;
    }

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

void OxideQQuickWebViewPrivate::completeConstruction() {
  Q_Q(OxideQQuickWebView);

  Q_ASSERT(construct_props_.data());

  if (construct_props_->new_view_request) {
    proxy_.reset(oxide::qt::WebViewProxy::create(
        this, contents_view_.data(), q,
        find_controller_.data(),
        security_status_.data(),
        construct_props_->new_view_request));
  }

  if (!proxy_) {
    construct_props_->new_view_request = nullptr;
    proxy_.reset(oxide::qt::WebViewProxy::create(
        this, contents_view_.data(), q,
        find_controller_.data(),
        security_status_.data(),
        construct_props_->context,
        construct_props_->incognito,
        construct_props_->restore_state,
        construct_props_->restore_type));
  }

  proxy_->messageHandlers().swap(construct_props_->message_handlers);

  proxy_->setLocationBarHeight(construct_props_->location_bar_height);
  proxy_->setLocationBarMode(construct_props_->location_bar_mode);
  proxy_->setLocationBarAnimated(construct_props_->location_bar_animated);

  if (!construct_props_->new_view_request) {
    if (construct_props_->load_html) {
      proxy_->loadHtml(construct_props_->html, construct_props_->url);
    } else if (!construct_props_->url.isEmpty()) {
      proxy_->setUrl(construct_props_->url);
    }
  }

  proxy_->setFullscreen(construct_props_->fullscreen);

  if (construct_props_->preferences) {
    proxy_->setPreferences(construct_props_->preferences);
  }

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

void OxideQQuickWebViewPrivate::didUpdatePaintNode() {
  if (received_new_compositor_frame_) {
    received_new_compositor_frame_ = false;
    proxy_->didCommitCompositorFrame();
  }
}

void OxideQQuickWebViewPrivate::screenChanged(QScreen* screen) {
  screenChangedHelper(screen);

  if (!proxy_) {
    return;
  }

  proxy_->screenUpdated();
}

void OxideQQuickWebViewPrivate::screenChangedHelper(QScreen* screen) {
  Q_Q(OxideQQuickWebView);

  if (screen_) {
    screen_->disconnect(q);
  }
  screen_ = screen;
  if (screen_) {
    screen_->setOrientationUpdateMask(
        Qt::PortraitOrientation |
        Qt::InvertedPortraitOrientation |
        Qt::LandscapeOrientation |
        Qt::InvertedLandscapeOrientation);
    q->connect(screen_, SIGNAL(virtualGeometryChanged(const QRect&)),
               SLOT(screenGeometryChanged(const QRect&)));
    q->connect(screen_, SIGNAL(geometryChanged(const QRect&)),
               SLOT(screenGeometryChanged(const QRect&)));
    q->connect(screen_, SIGNAL(orientationChanged(Qt::ScreenOrientation)),
               SLOT(screenOrientationChanged(Qt::ScreenOrientation)));
    q->connect(screen_, SIGNAL(primaryOrientationChanged(Qt::ScreenOrientation)),
               SLOT(screenOrientationChanged(Qt::ScreenOrientation)));
  }
}

void OxideQQuickWebViewPrivate::windowChangedHelper(QQuickWindow* window) {
  Q_Q(OxideQQuickWebView);

  if (window_) {
    window_->disconnect(q);
  }
  window_ = window;
  if (window_) {
    q->connect(window_, SIGNAL(screenChanged(QScreen*)),
               SLOT(screenChanged(QScreen*)));
  }

  screenChangedHelper(window_ ? window_->screen() : nullptr);

  if (!proxy_) {
    return;
  }

  proxy_->screenUpdated();
  proxy_->wasResized();
}

void OxideQQuickWebViewPrivate::screenGeometryChanged(const QRect& rect) {
  if (!proxy_) {
    return;
  }

  proxy_->screenUpdated();
}

void OxideQQuickWebViewPrivate::screenOrientationChanged(
    Qt::ScreenOrientation orientation) {
  if (!proxy_) {
    return;
  }

  proxy_->screenUpdated();
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

int OxideQQuickWebViewPrivate::locationBarHeight() {
  if (!proxy_) {
    return construct_props_->location_bar_height;
  }

  return proxy_->locationBarHeight();
}

void OxideQQuickWebViewPrivate::setLocationBarHeight(int height) {
  if (!proxy_) {
    construct_props_->location_bar_height = height;
  } else {
    proxy_->setLocationBarHeight(height);
  }
}

oxide::qt::LocationBarMode OxideQQuickWebViewPrivate::locationBarMode() const {
  if (!proxy_) {
    return construct_props_->location_bar_mode;
  }

  return proxy_->locationBarMode();
}

void OxideQQuickWebViewPrivate::setLocationBarMode(
    oxide::qt::LocationBarMode mode) {
  if (!proxy_) {
    construct_props_->location_bar_mode = mode;
  } else {
    proxy_->setLocationBarMode(mode);
  }
}

bool OxideQQuickWebViewPrivate::locationBarAnimated() const {
  if (!proxy_) {
    return construct_props_->location_bar_animated;
  }

  return proxy_->locationBarAnimated();
}

void OxideQQuickWebViewPrivate::setLocationBarAnimated(bool animated) {
  if (!proxy_) {
    construct_props_->location_bar_animated = animated;
  } else {
    proxy_->setLocationBarAnimated(animated);
  }
}

int OxideQQuickWebViewPrivate::locationBarOffsetPix() {
  if (!proxy_) {
    return 0;
  }

  return proxy_->locationBarOffsetPix();
}

int OxideQQuickWebViewPrivate::locationBarContentOffsetPix() {
  if (!proxy_) {
    return 0;
  }

  return proxy_->locationBarContentOffsetPix();
}

void OxideQQuickWebViewPrivate::locationBarShow(bool animate) {
  if (!proxy_) {
    return;
  }

  proxy_->locationBarShow(animate);
}

void OxideQQuickWebViewPrivate::locationBarHide(bool animate) {
  if (!proxy_) {
    return;
  }

  proxy_->locationBarHide(animate);
}

int OxideQQuickWebViewPrivate::getNavigationEntryCount() const {
  if (!proxy_) {
    return 0;
  }

  return proxy_->getNavigationEntryCount();
}

int OxideQQuickWebViewPrivate::getNavigationCurrentEntryIndex() const {
  if (!proxy_) {
    return -1;
  }

  return proxy_->getNavigationCurrentEntryIndex();
}

void OxideQQuickWebViewPrivate::setNavigationCurrentEntryIndex(int index) {
  proxy_->setNavigationCurrentEntryIndex(index);
}

int OxideQQuickWebViewPrivate::getNavigationEntryUniqueID(int index) const {
  return proxy_->getNavigationEntryUniqueID(index);
}

QUrl OxideQQuickWebViewPrivate::getNavigationEntryUrl(int index) const {
  return proxy_->getNavigationEntryUrl(index);
}

QString OxideQQuickWebViewPrivate::getNavigationEntryTitle(int index) const {
  return proxy_->getNavigationEntryTitle(index);
}

QDateTime OxideQQuickWebViewPrivate::getNavigationEntryTimestamp(
    int index) const {
  return proxy_->getNavigationEntryTimestamp(index);
}

void OxideQQuickWebView::connectNotify(const QMetaMethod& signal) {
  Q_D(OxideQQuickWebView);

  Q_ASSERT(thread() == QThread::currentThread());

#define VIEW_SIGNAL(sig) QMetaMethod::fromSignal(&OxideQQuickWebView::sig)
  if (signal == VIEW_SIGNAL(newViewRequested) && d->proxy_) {
    d->proxy_->updateWebPreferences();
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
    d->proxy_->updateWebPreferences();
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

  OxideQQuickWebContext* context = nullptr;
  if (d->construct_props_->context) {
    context = d->construct_props_->context;
  }

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

  switch (query) {
    case Qt::ImEnabled:
      return (flags() & QQuickItem::ItemAcceptsInputMethod) != 0;
    default:
      return d->proxy_ ? d->proxy_->inputMethodQuery(query) : QVariant();
  }
}

void OxideQQuickWebView::itemChange(QQuickItem::ItemChange change,
                                    const QQuickItem::ItemChangeData& value) {
  Q_D(OxideQQuickWebView);

  QQuickItem::itemChange(change, value);

  if (!d->proxy_) {
    return;
  }

  if (change == QQuickItem::ItemVisibleHasChanged) {
    d->proxy_->visibilityChanged();
  }
}

void OxideQQuickWebView::keyPressEvent(QKeyEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::keyPressEvent(event);

  if (d->handling_unhandled_key_event_ || !d->proxy_) {
    return;
  }

  d->proxy_->handleKeyEvent(event);
}

void OxideQQuickWebView::keyReleaseEvent(QKeyEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::keyReleaseEvent(event);

  if (d->handling_unhandled_key_event_ || !d->proxy_) {
    return;
  }

  d->proxy_->handleKeyEvent(event);
}

void OxideQQuickWebView::inputMethodEvent(QInputMethodEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::inputMethodEvent(event);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->handleInputMethodEvent(event);
}

void OxideQQuickWebView::focusInEvent(QFocusEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::focusInEvent(event);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->handleFocusEvent(event);
}

void OxideQQuickWebView::focusOutEvent(QFocusEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::focusOutEvent(event);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->handleFocusEvent(event);
}

void OxideQQuickWebView::mousePressEvent(QMouseEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::mousePressEvent(event);

  if (!d->proxy_) {
    return;
  }

  forceActiveFocus();
  d->proxy_->handleMouseEvent(event);
}

void OxideQQuickWebView::mouseMoveEvent(QMouseEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::mouseMoveEvent(event);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->handleMouseEvent(event);
}

void OxideQQuickWebView::mouseReleaseEvent(QMouseEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::mouseReleaseEvent(event);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->handleMouseEvent(event);
}

void OxideQQuickWebView::wheelEvent(QWheelEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::wheelEvent(event);

  if (!d->proxy_) {
    return;
  }

  QPointF window_pos = mapToScene(event->posF());
  d->proxy_->handleWheelEvent(event, window_pos.toPoint());
}

void OxideQQuickWebView::touchEvent(QTouchEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::touchEvent(event);

  if (!d->proxy_) {
    return;
  }

  if (event->type() == QEvent::TouchBegin) {
    forceActiveFocus();
  }
  d->proxy_->handleTouchEvent(event);
}

void OxideQQuickWebView::hoverEnterEvent(QHoverEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::hoverEnterEvent(event);

  if (!d->proxy_) {
    return;
  }

  QPointF window_pos = mapToScene(event->posF());
  d->proxy_->handleHoverEvent(event,
                              window_pos.toPoint(),
                              (window_pos + window()->position()).toPoint());
}

void OxideQQuickWebView::hoverMoveEvent(QHoverEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::hoverMoveEvent(event);

  if (!d->proxy_) {
    return;
  }

  QPointF window_pos = mapToScene(event->posF());
  d->proxy_->handleHoverEvent(event,
                              window_pos.toPoint(),
                              (window_pos + window()->position()).toPoint());
}

void OxideQQuickWebView::hoverLeaveEvent(QHoverEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::hoverLeaveEvent(event);

  if (!d->proxy_) {
    return;
  }

  QPointF window_pos = mapToScene(event->posF());
  d->proxy_->handleHoverEvent(event,
                              window_pos.toPoint(),
                              (window_pos + window()->position()).toPoint());
}

void OxideQQuickWebView::dragEnterEvent(QDragEnterEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::dragEnterEvent(event);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->handleDragEnterEvent(event);
}

void OxideQQuickWebView::dragMoveEvent(QDragMoveEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::dragMoveEvent(event);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->handleDragMoveEvent(event);
}

void OxideQQuickWebView::dragLeaveEvent(QDragLeaveEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::dragLeaveEvent(event);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->handleDragLeaveEvent(event);
}

void OxideQQuickWebView::dropEvent(QDropEvent* event) {
  Q_D(OxideQQuickWebView);

  QQuickItem::dropEvent(event);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->handleDropEvent(event);
}

void OxideQQuickWebView::geometryChanged(const QRectF& newGeometry,
                                         const QRectF& oldGeometry) {
  Q_D(OxideQQuickWebView);

  QQuickItem::geometryChanged(newGeometry, oldGeometry);

  if (d->proxy_ && window()) {
    d->proxy_->wasResized();
  }
}

QSGNode* OxideQQuickWebView::updatePaintNode(
    QSGNode* oldNode,
    UpdatePaintNodeData * updatePaintNodeData) {
  Q_UNUSED(updatePaintNodeData);
  Q_D(OxideQQuickWebView);

  UpdatePaintNodeScope scope(d);

  oxide::qt::CompositorFrameHandle::Type type =
      oxide::qt::CompositorFrameHandle::TYPE_INVALID;
  QSharedPointer<oxide::qt::CompositorFrameHandle> handle;

  if (d->proxy_) {
    handle = d->proxy_->compositorFrameHandle();
    type = handle->GetType();
  }

  Q_ASSERT(!d->received_new_compositor_frame_ ||
           (d->received_new_compositor_frame_ && !d->frame_evicted_));

  if (type != d->last_composited_frame_type_) {
    delete oldNode;
    oldNode = nullptr;
  }

  d->last_composited_frame_type_ = type;

  if (d->frame_evicted_) {
    delete oldNode;
    return nullptr;
  }

  if (type == oxide::qt::CompositorFrameHandle::TYPE_ACCELERATED) {
    AcceleratedFrameNode* node = static_cast<AcceleratedFrameNode *>(oldNode);
    if (!node) {
      node = new AcceleratedFrameNode(this);
    }

    if (d->received_new_compositor_frame_ || !oldNode) {
      node->updateNode(handle);
    }

    return node;
  }

  if (type == oxide::qt::CompositorFrameHandle::TYPE_IMAGE) {
    ImageFrameNode* node = static_cast<ImageFrameNode *>(oldNode);
    if (!node) {
      node = new ImageFrameNode();
    }

    if (d->received_new_compositor_frame_ || !oldNode) {
      node->updateNode(handle);
    }

    return node;
  }

  if (type == oxide::qt::CompositorFrameHandle::TYPE_SOFTWARE) {
    SoftwareFrameNode* node = static_cast<SoftwareFrameNode *>(oldNode);
    if (!node) {
      node = new SoftwareFrameNode(this);
    }

    if (d->received_new_compositor_frame_ || !oldNode) {
      node->updateNode(handle);
    }

    return node;
  }

  Q_ASSERT(type == oxide::qt::CompositorFrameHandle::TYPE_INVALID);

  SoftwareFrameNode* node = static_cast<SoftwareFrameNode *>(oldNode);
  if (!node) {
    node = new SoftwareFrameNode(this);
  }

  QRectF rect(QPointF(0, 0), QSizeF(width(), height()));

  if (!oldNode || rect != node->rect()) {
    QImage blank(qRound(rect.width()),
                 qRound(rect.height()),
                 QImage::Format_ARGB32);
    blank.fill(Qt::white);
    node->setImage(blank);
  }

  return node;
}

OxideQQuickWebView::OxideQQuickWebView(QQuickItem* parent)
    : QQuickItem(parent),
      d_ptr(new OxideQQuickWebViewPrivate(this)) {
  oxide::qquick::EnsureChromiumStarted();

  Q_D(OxideQQuickWebView);

  setFlags(QQuickItem::ItemClipsChildrenToShape |
           QQuickItem::ItemHasContents |
           QQuickItem::ItemIsFocusScope |
           QQuickItem::ItemAcceptsDrops);
  setAcceptedMouseButtons(Qt::AllButtons);
  setAcceptHoverEvents(true);

  connect(this, SIGNAL(windowChanged(QQuickWindow*)),
          this, SLOT(windowChangedHelper(QQuickWindow*)));
}

OxideQQuickWebView::~OxideQQuickWebView() {
  Q_D(OxideQQuickWebView);

  disconnect(this, SIGNAL(windowChanged(QQuickWindow*)),
             this, SLOT(windowChangedHelper(QQuickWindow*)));
  if (d->window_) {
    d->window_->disconnect(this);
  }
  if (d->screen_) {
    d->screen_->disconnect(this);
  }

  d->detachContextSignals(
      OxideQQuickWebContextPrivate::get(
        qobject_cast<OxideQQuickWebContext*>(d->contextHandle())));

  // Do this before our d_ptr is cleared, as these call back in to us
  // when they are deleted
  while (d->messageHandlers().size() > 0) {
    delete d->messageHandlers().at(0);
  }

  d->proxy_->teardownFrameTree();
}

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

QString OxideQQuickWebView::title() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return QString();
  }

  return d->proxy_->title();
}

QUrl OxideQQuickWebView::icon() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return QUrl();
  }

  return d->proxy_->favIconUrl();
}

bool OxideQQuickWebView::canGoBack() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return false;
  }

  return d->proxy_->canGoBack();
}

bool OxideQQuickWebView::canGoForward() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return false;
  }

  return d->proxy_->canGoForward();
}

bool OxideQQuickWebView::incognito() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return d->construct_props_->incognito;
  }

  return d->proxy_->incognito();
}

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

bool OxideQQuickWebView::loading() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return false;
  }

  return d->proxy_->loading();
}

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

int OxideQQuickWebView::loadProgress() const {
  Q_D(const OxideQQuickWebView);

  return d->load_progress_;
}

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

QQmlListProperty<OxideQQuickScriptMessageHandler>
OxideQQuickWebView::messageHandlers() {
  return QQmlListProperty<OxideQQuickScriptMessageHandler>(
      this, nullptr,
      OxideQQuickWebViewPrivate::messageHandler_append,
      OxideQQuickWebViewPrivate::messageHandler_count,
      OxideQQuickWebViewPrivate::messageHandler_at,
      OxideQQuickWebViewPrivate::messageHandler_clear);
}

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

qreal OxideQQuickWebView::viewportWidth() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return 0.f;
  }

  return const_cast<OxideQQuickWebViewPrivate*>(
      d)->proxy_->compositorFrameViewportSizePix().width();
}

qreal OxideQQuickWebView::viewportHeight() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return 0.f;
  }

  return const_cast<OxideQQuickWebViewPrivate*>(
      d)->proxy_->compositorFrameViewportSizePix().height();
}

qreal OxideQQuickWebView::contentWidth() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return 0.f;
  }

  return const_cast<OxideQQuickWebViewPrivate*>(
      d)->proxy_->compositorFrameContentSizePix().width();
}

qreal OxideQQuickWebView::contentHeight() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return 0.f;
  }

  return const_cast<OxideQQuickWebViewPrivate*>(
      d)->proxy_->compositorFrameContentSizePix().height();
}

qreal OxideQQuickWebView::contentX() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return 0.f;
  }

  return const_cast<OxideQQuickWebViewPrivate*>(
      d)->proxy_->compositorFrameScrollOffsetPix().x();
}

qreal OxideQQuickWebView::contentY() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return 0.f;
  }

  return const_cast<OxideQQuickWebViewPrivate*>(
      d)->proxy_->compositorFrameScrollOffsetPix().y();
}

QQmlComponent* OxideQQuickWebView::contextMenu() const {
  Q_D(const OxideQQuickWebView);

  return d->contents_view_->contextMenu();
}

void OxideQQuickWebView::setContextMenu(QQmlComponent* context_menu) {
  Q_D(OxideQQuickWebView);

  if (d->contents_view_->contextMenu() == context_menu) {
    return;
  }

  d->contents_view_->setContextMenu(context_menu);
  emit contextMenuChanged();
}

QQmlComponent* OxideQQuickWebView::popupMenu() const {
  Q_D(const OxideQQuickWebView);

  return d->contents_view_->popupMenu();
}

void OxideQQuickWebView::setPopupMenu(QQmlComponent* popup_menu) {
  Q_D(OxideQQuickWebView);

  if (d->contents_view_->popupMenu() == popup_menu) {
    return;
  }

  d->contents_view_->popupMenu();
  emit popupMenuChanged();
}

QQmlComponent* OxideQQuickWebView::alertDialog() const {
  Q_D(const OxideQQuickWebView);

  return d->alert_dialog_;
}

void OxideQQuickWebView::setAlertDialog(QQmlComponent* alert_dialog) {
  Q_D(OxideQQuickWebView);

  if (d->alert_dialog_ == alert_dialog) {
    return;
  }

  d->alert_dialog_ = alert_dialog;
  emit alertDialogChanged();
}

QQmlComponent* OxideQQuickWebView::confirmDialog() const {
  Q_D(const OxideQQuickWebView);

  return d->confirm_dialog_;
}

void OxideQQuickWebView::setConfirmDialog(QQmlComponent* confirm_dialog) {
  Q_D(OxideQQuickWebView);

  if (d->confirm_dialog_ == confirm_dialog) {
    return;
  }

  d->confirm_dialog_ = confirm_dialog;
  emit confirmDialogChanged();
}

QQmlComponent* OxideQQuickWebView::promptDialog() const {
  Q_D(const OxideQQuickWebView);

  return d->prompt_dialog_;
}

void OxideQQuickWebView::setPromptDialog(QQmlComponent* prompt_dialog) {
  Q_D(OxideQQuickWebView);

  if (d->prompt_dialog_ == prompt_dialog) {
    return;
  }

  d->prompt_dialog_ = prompt_dialog;
  emit promptDialogChanged();
}

QQmlComponent* OxideQQuickWebView::beforeUnloadDialog() const {
  Q_D(const OxideQQuickWebView);

  return d->before_unload_dialog_;
}

void OxideQQuickWebView::setBeforeUnloadDialog(
    QQmlComponent* before_unload_dialog) {
  Q_D(OxideQQuickWebView);

  if (d->before_unload_dialog_ == before_unload_dialog) {
    return;
  }

  d->before_unload_dialog_ = before_unload_dialog;
  emit beforeUnloadDialogChanged();
}

QQmlComponent* OxideQQuickWebView::filePicker() const {
  Q_D(const OxideQQuickWebView);

  return d->file_picker_;
}

void OxideQQuickWebView::setFilePicker(QQmlComponent* file_picker) {
  Q_D(OxideQQuickWebView);

  if (d->file_picker_ == file_picker) {
    return;
  }

  d->file_picker_ = file_picker;
  emit filePickerChanged();
}

OxideQQuickWebContext* OxideQQuickWebView::context() const {
  Q_D(const OxideQQuickWebView);

  QObject* c = d->contextHandle();
  if (!c) {
    return nullptr;
  }

  return qobject_cast<OxideQQuickWebContext*>(c);
}

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

OxideQWebPreferences* OxideQQuickWebView::preferences() {
  Q_D(OxideQQuickWebView);

  if (!d->proxy_) {
    if (!d->construct_props_->preferences) {
      d->construct_props_->preferences = new OxideQWebPreferences(this);
    }
    return d->construct_props_->preferences;
  }

  return d->proxy_->preferences();
}

void OxideQQuickWebView::setPreferences(OxideQWebPreferences* prefs) {
  Q_D(OxideQQuickWebView);

  if (prefs == this->preferences()) {
    return;
  }

  if (!d->proxy_) {
    d->construct_props_->preferences = prefs;
  } else {
    d->proxy_->setPreferences(prefs);
  }

  emit preferencesChanged();
}

OxideQQuickNavigationHistory* OxideQQuickWebView::navigationHistory() {
  Q_D(OxideQQuickWebView);

  return &d->navigation_history_;
}

OxideQSecurityStatus* OxideQQuickWebView::securityStatus() {
  Q_D(OxideQQuickWebView);

  return d->security_status_.data();
}

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

// This exists purely to remove a moc warning. We don't store this request
// anywhere, it's only a transient object and I can't think of any possible
// reason why anybody would want to read it back
OxideQNewViewRequest* OxideQQuickWebView::request() const {
  return nullptr;
}

void OxideQQuickWebView::setRequest(OxideQNewViewRequest* request) {
  Q_D(OxideQQuickWebView);

  if (d->proxy_) {
    qWarning() <<
        "OxideQQuickWebView: request must be provided during construction";
    return;
  }

  d->construct_props_->new_view_request = request;
}

// This exists purely to remove a moc warning. We don't store this initial state
// anywhere, it's only a transient blob and I can't think of any possible
// reason why anybody would want to read it back
QString OxideQQuickWebView::restoreState() const {
  return QString();
}

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

// This exists purely to remove a moc warning. We don't store this restore type
// anywhere, it's only a transient property and I can't think of any possible
// reason why anybody would want to read it back
OxideQQuickWebView::RestoreType OxideQQuickWebView::restoreType() const {
  Q_D(const OxideQQuickWebView);

  return RestoreLastSessionExitedCleanly;
}

void OxideQQuickWebView::setRestoreType(OxideQQuickWebView::RestoreType type) {
  Q_D(OxideQQuickWebView);

  if (d->proxy_) {
    qWarning() <<
        "OxideQQuickWebView: restoreType must be provided during construction";
    return;
  }

  Q_STATIC_ASSERT(
      RestoreCurrentSession ==
        static_cast<RestoreType>(oxide::qt::RESTORE_CURRENT_SESSION));
  Q_STATIC_ASSERT(
      RestoreLastSessionExitedCleanly ==
        static_cast<RestoreType>(oxide::qt::RESTORE_LAST_SESSION_EXITED_CLEANLY));
  Q_STATIC_ASSERT(
      RestoreLastSessionCrashed ==
        static_cast<RestoreType>(oxide::qt::RESTORE_LAST_SESSION_CRASHED));

  d->construct_props_->restore_type = static_cast<oxide::qt::RestoreType>(type);
}

QString OxideQQuickWebView::currentState() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return QString();
  }

  // Encode the current state in base64 so it can be safely passed around
  // as a string (QML doesn’t know of byte arrays)
  return QString::fromLocal8Bit(d->proxy_->currentState().toBase64());
}

OxideQQuickLocationBarController* OxideQQuickWebView::locationBarController() {
  Q_D(OxideQQuickWebView);

  if (!d->location_bar_controller_) {
    d->location_bar_controller_.reset(
        new OxideQQuickLocationBarController(this));
  }

  return d->location_bar_controller_.data();
}

OxideQQuickWebView::WebProcessStatus OxideQQuickWebView::webProcessStatus() const {
  Q_D(const OxideQQuickWebView);

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

QUrl OxideQQuickWebView::hoveredUrl() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return QUrl();
  }

  return d->proxy_->targetUrl();
}

OxideQQuickWebView::EditCapabilities OxideQQuickWebView::editingCapabilities() const {
  Q_D(const OxideQQuickWebView);

  if (!d->proxy_) {
    return NoCapability;
  }

  oxide::qt::EditCapabilityFlags flags = d->proxy_->editFlags();
  return static_cast<EditCapabilities>(flags);
}

// static
OxideQQuickWebViewAttached* OxideQQuickWebView::qmlAttachedProperties(
    QObject* object) {
  return new OxideQQuickWebViewAttached(object);
}

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

void OxideQQuickWebView::goBack() {
  Q_D(OxideQQuickWebView);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->goBack();
}

void OxideQQuickWebView::goForward() {
  Q_D(OxideQQuickWebView);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->goForward();
}

void OxideQQuickWebView::stop() {
  Q_D(OxideQQuickWebView);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->stop();
}

void OxideQQuickWebView::reload() {
  Q_D(OxideQQuickWebView);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->reload();
}

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

void OxideQQuickWebView::setCanTemporarilyDisplayInsecureContent(bool allow) {
  Q_D(OxideQQuickWebView);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->setCanTemporarilyDisplayInsecureContent(allow);
}

void OxideQQuickWebView::setCanTemporarilyRunInsecureContent(bool allow) {
  Q_D(OxideQQuickWebView);

  if (!d->proxy_) {
    return;
  }

  d->proxy_->setCanTemporarilyRunInsecureContent(allow);
}

void OxideQQuickWebView::prepareToClose() {
  Q_D(OxideQQuickWebView);

  if (!d->proxy_) {
    QCoreApplication::postEvent(this,
                                new QEvent(GetPrepareToCloseBypassEventType()));
    return;
  }

  d->proxy_->prepareToClose();
}

OxideQFindController* OxideQQuickWebView::findController() const {
  Q_D(const OxideQQuickWebView);

  return d->find_controller_.data();
}

OxideQQuickTouchSelectionController* OxideQQuickWebView::touchSelectionController() {
  Q_D(OxideQQuickWebView);

  if (!d->touch_selection_controller_) {
    d->touch_selection_controller_.reset(
        new OxideQQuickTouchSelectionController(this));
  }

  return d->touch_selection_controller_.data();
}

#include "moc_oxideqquickwebview.cpp"
