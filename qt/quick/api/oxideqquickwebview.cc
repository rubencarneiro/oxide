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

#include "oxideqquickwebview_p.h"
#include "oxideqquickwebview_p_p.h"

#include <QKeyEvent>
#include <QMetaMethod>
#include <QPointF>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QRect>
#include <QRectF>
#include <QSizeF>
#include <QSize>
#include <QtQml>

#include "qt/core/api/oxideqpermissionrequest.h"
#include "qt/quick/oxide_qquick_alert_dialog_delegate.h"
#include "qt/quick/oxide_qquick_before_unload_dialog_delegate.h"
#include "qt/quick/oxide_qquick_confirm_dialog_delegate.h"
#include "qt/quick/oxide_qquick_file_picker_delegate.h"
#include "qt/quick/oxide_qquick_prompt_dialog_delegate.h"
#include "qt/quick/oxide_qquick_render_view_item.h"
#include "qt/quick/oxide_qquick_web_popup_menu_delegate.h"

#include "oxideqquickscriptmessagehandler_p.h"
#include "oxideqquickscriptmessagehandler_p_p.h"
#include "oxideqquickwebcontext_p.h"
#include "oxideqquickwebcontext_p_p.h"
#include "oxideqquickwebframe_p.h"
#include "oxideqquickwebframe_p_p.h"

QT_USE_NAMESPACE

OxideQQuickWebViewAttached::OxideQQuickWebViewAttached(QObject* parent) :
    QObject(parent),
    view_(NULL) {}

OxideQQuickWebViewAttached::~OxideQQuickWebViewAttached() {}

OxideQQuickWebView* OxideQQuickWebViewAttached::view() const {
  return view_;
}

void OxideQQuickWebViewAttached::setView(OxideQQuickWebView* view) {
  view_ = view;
}

OxideQQuickWebViewPrivate::OxideQQuickWebViewPrivate(
    OxideQQuickWebView* view) :
    oxide::qt::WebViewAdapter(view),
    load_progress_(0),
    constructed_(false),
    navigation_history_(view),
    popup_menu_(NULL),
    alert_dialog_(NULL),
    confirm_dialog_(NULL),
    prompt_dialog_(NULL),
    before_unload_dialog_(NULL),
    file_picker_(NULL) {}

oxide::qt::WebPopupMenuDelegate*
OxideQQuickWebViewPrivate::CreateWebPopupMenuDelegate() {
  Q_Q(OxideQQuickWebView);

  return new oxide::qquick::WebPopupMenuDelegate(q);
}

oxide::qt::JavaScriptDialogDelegate*
OxideQQuickWebViewPrivate::CreateJavaScriptDialogDelegate(
    oxide::qt::JavaScriptDialogDelegate::Type type) {
  Q_Q(OxideQQuickWebView);

  switch (type) {
  case oxide::qt::JavaScriptDialogDelegate::TypeAlert:
    return new oxide::qquick::AlertDialogDelegate(q);
  case oxide::qt::JavaScriptDialogDelegate::TypeConfirm:
    return new oxide::qquick::ConfirmDialogDelegate(q);
  case oxide::qt::JavaScriptDialogDelegate::TypePrompt:
    return new oxide::qquick::PromptDialogDelegate(q);
  default:
    Q_UNREACHABLE();
  }
}

oxide::qt::JavaScriptDialogDelegate*
OxideQQuickWebViewPrivate::CreateBeforeUnloadDialogDelegate() {
  Q_Q(OxideQQuickWebView);

  return new oxide::qquick::BeforeUnloadDialogDelegate(q);
}

oxide::qt::FilePickerDelegate*
OxideQQuickWebViewPrivate::CreateFilePickerDelegate() {
  Q_Q(OxideQQuickWebView);

  return new oxide::qquick::FilePickerDelegate(q);
}

void OxideQQuickWebViewPrivate::OnInitialized(
    bool orig_incognito,
    oxide::qt::WebContextAdapter* orig_context) {
  Q_Q(OxideQQuickWebView);

  // Make the webview the QObject parent of the new root frame,
  // to stop Qml from collecting the frame tree
  q->rootFrame()->setParent(q);

  // Initialization created the root frame. This is the only time
  // this is emitted
  emit q->rootFrameChanged();

  if (orig_incognito != incognito()) {
    emit q->incognitoChanged();
  }
  if (orig_context != context()) {
    detachContextSignals(static_cast<OxideQQuickWebContextPrivate *>(orig_context));
    attachContextSignals(static_cast<OxideQQuickWebContextPrivate *>(context()));

    emit q->contextChanged();
  }
}

void OxideQQuickWebViewPrivate::URLChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->urlChanged();
}

void OxideQQuickWebViewPrivate::TitleChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->titleChanged();
}

void OxideQQuickWebViewPrivate::IconChanged(QUrl icon) {
  Q_Q(OxideQQuickWebView);

  if (icon != icon_) {
    icon_ = icon;
    emit q->iconChanged();
  }
}

void OxideQQuickWebViewPrivate::CommandsUpdated() {
  Q_Q(OxideQQuickWebView);

  emit q->navigationHistoryChanged();
}

void OxideQQuickWebViewPrivate::LoadProgressChanged(double progress) {
  Q_Q(OxideQQuickWebView);

  load_progress_ = progress * 100;
  emit q->loadProgressChanged();
}

void OxideQQuickWebViewPrivate::LoadEvent(OxideQLoadEvent* event) {
  Q_Q(OxideQQuickWebView);

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

oxide::qt::WebFrameAdapter* OxideQQuickWebViewPrivate::CreateWebFrame() {
  OxideQQuickWebFrame* frame = new OxideQQuickWebFrame();
  QQmlEngine::setObjectOwnership(frame, QQmlEngine::CppOwnership);
  return OxideQQuickWebFramePrivate::get(frame);
}

QRect OxideQQuickWebViewPrivate::GetContainerBounds() {
  Q_Q(OxideQQuickWebView);

  QPointF pos(q->mapToScene(QPointF(0,0)));
  if (q->window()) {
    // We could be called before being added to a scene
    pos += q->window()->position();
  }

  return QRectF(pos.x(), pos.y(),
                q->width(), q->height()).toRect();
}

bool OxideQQuickWebViewPrivate::IsVisible() const {
  Q_Q(const OxideQQuickWebView);

  return q->isVisible();
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

void OxideQQuickWebViewPrivate::OnWebPreferencesChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->preferencesChanged();
}

void OxideQQuickWebViewPrivate::FrameAdded(
    oxide::qt::WebFrameAdapter* frame) {
  Q_Q(OxideQQuickWebView);

  emit q->frameAdded(adapterToQObject<OxideQQuickWebFrame>(frame));
}

void OxideQQuickWebViewPrivate::FrameRemoved(
    oxide::qt::WebFrameAdapter* frame) {
  Q_Q(OxideQQuickWebView);

  emit q->frameRemoved(adapterToQObject<OxideQQuickWebFrame>(frame));
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

  QQmlEngine::setObjectOwnership(request, QQmlEngine::JavaScriptOwnership);
  emit q->geolocationPermissionRequested(request);
}

void OxideQQuickWebViewPrivate::HandleKeyboardEvent(QKeyEvent* event) {
  Q_Q(OxideQQuickWebView);

  QQuickWindow* w = q->window();
  if (!w) {
    return;
  }

  w->sendEvent(q, event);
}

void OxideQQuickWebViewPrivate::completeConstruction() {
  Q_Q(OxideQQuickWebView);

  if (!context()) {
    OxideQQuickWebContext* c = OxideQQuickWebContext::defaultContext(true);
    setContext(OxideQQuickWebContextPrivate::get(c));
  }

  init();
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
        static_cast<OxideQQuickWebView *>(prop->object));

  return p->message_handlers().size();
}

// static
OxideQQuickScriptMessageHandler* OxideQQuickWebViewPrivate::messageHandler_at(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop,
    int index) {
  OxideQQuickWebViewPrivate* p = OxideQQuickWebViewPrivate::get(
        static_cast<OxideQQuickWebView *>(prop->object));

  if (index >= p->message_handlers().size()) {
    return NULL;
  }

  return adapterToQObject<OxideQQuickScriptMessageHandler>(
      p->message_handlers().at(index));
}

// static
void OxideQQuickWebViewPrivate::messageHandler_clear(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop) {
  OxideQQuickWebView* web_view =
      static_cast<OxideQQuickWebView *>(prop->object);
  OxideQQuickWebViewPrivate* p = OxideQQuickWebViewPrivate::get(web_view);

  while (p->message_handlers().size() > 0) {
    web_view->removeMessageHandler(
        adapterToQObject<OxideQQuickScriptMessageHandler>(
          p->message_handlers().at(0)));
  }
}

void OxideQQuickWebViewPrivate::contextConstructed() {
  if (constructed_) {
    completeConstruction();
  }
}

void OxideQQuickWebViewPrivate::contextWillBeDestroyed() {
  Q_Q(OxideQQuickWebView);

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

  QObject::connect(context, SIGNAL(willBeDestroyed()),
                   q, SLOT(contextWillBeDestroyed()));
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
  QObject::disconnect(context, SIGNAL(willBeDestroyed()),
                      q, SLOT(contextWillBeDestroyed()));
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

void OxideQQuickWebView::connectNotify(const QMetaMethod& signal) {
  Q_D(OxideQQuickWebView);

  Q_ASSERT(thread() == QThread::currentThread());

  if (signal == QMetaMethod::fromSignal(
          &OxideQQuickWebView::newViewRequested)) {
    d->updateWebPreferences();
  }
}

void OxideQQuickWebView::disconnectNotify(const QMetaMethod& signal) {
  Q_D(OxideQQuickWebView);

  Q_ASSERT(thread() == QThread::currentThread());

  if (signal == QMetaMethod::fromSignal(
          &OxideQQuickWebView::newViewRequested) ||
      !signal.isValid()) {
    d->updateWebPreferences();
  }
}

void OxideQQuickWebView::geometryChanged(const QRectF& newGeometry,
                                         const QRectF& oldGeometry) {
  Q_D(OxideQQuickWebView);

  QQuickItem::geometryChanged(newGeometry, oldGeometry);

  if (d->isInitialized()) {
    d->updateSize(newGeometry.size().toSize());
  }
}

void OxideQQuickWebView::itemChange(QQuickItem::ItemChange change,
                                    const QQuickItem::ItemChangeData& value) {
  Q_D(OxideQQuickWebView);

  QQuickItem::itemChange(change, value);

  if (!d->isInitialized()) {
    return;
  }

  if (change == QQuickItem::ItemVisibleHasChanged) {
    d->updateVisibility(value.boolValue);
  }
}

OxideQQuickWebView::OxideQQuickWebView(QQuickItem* parent) :
    QQuickItem(parent) {
  // WebView instantiates NotificationRegistrar, which starts
  // NotificationService, which uses LazyInstance. Start Chromium now
  // else we'll crash
  OxideQQuickWebContextPrivate::ensureChromiumStarted();
  d_ptr.reset(new OxideQQuickWebViewPrivate(this));

  setFlags(QQuickItem::ItemIsFocusScope);
}

OxideQQuickWebView::~OxideQQuickWebView() {
  Q_D(OxideQQuickWebView);

  d->detachContextSignals(
      static_cast<OxideQQuickWebContextPrivate *>(d->context()));

  // Do this before our d_ptr is cleared, as these call back in to us
  // when they are deleted
  while (d->message_handlers().size() > 0) {
    delete adapterToQObject<OxideQQuickScriptMessageHandler>(
        d->message_handlers().at(0));
  }
}

void OxideQQuickWebView::componentComplete() {
  Q_D(OxideQQuickWebView);

  Q_ASSERT(!d->constructed_);
  d->constructed_ = true;

  QQuickItem::componentComplete();

  if (!d->context() ||
      static_cast<OxideQQuickWebContextPrivate *>(d->context())->isConstructed()) {
    d->completeConstruction();
  }
}

QUrl OxideQQuickWebView::url() const {
  Q_D(const OxideQQuickWebView);

  return d->url();
}

void OxideQQuickWebView::setUrl(const QUrl& url) {
  Q_D(OxideQQuickWebView);

  d->setUrl(url);
}

QString OxideQQuickWebView::title() const {
  Q_D(const OxideQQuickWebView);

  return d->title();
}

QUrl OxideQQuickWebView::icon() const {
  Q_D(const OxideQQuickWebView);

  return d->icon_;
}

bool OxideQQuickWebView::canGoBack() const {
  Q_D(const OxideQQuickWebView);

  return d->canGoBack();
}

bool OxideQQuickWebView::canGoForward() const {
  Q_D(const OxideQQuickWebView);

  return d->canGoForward();
}

bool OxideQQuickWebView::incognito() const {
  Q_D(const OxideQQuickWebView);

  return d->incognito();
}

void OxideQQuickWebView::setIncognito(bool incognito) {
  Q_D(OxideQQuickWebView);

  if (incognito == d->incognito()) {
    return;
  }

  d->setIncognito(incognito);
  emit incognitoChanged();
}

bool OxideQQuickWebView::loading() const {
  Q_D(const OxideQQuickWebView);

  return d->loading();
}

bool OxideQQuickWebView::fullscreen() const {
  Q_D(const OxideQQuickWebView);

  return d->fullscreen();
}

void OxideQQuickWebView::setFullscreen(bool fullscreen) {
  Q_D(OxideQQuickWebView);

  if (fullscreen == d->fullscreen()) {
    return;
  }

  d->setFullscreen(fullscreen);
  emit fullscreenChanged();
}

int OxideQQuickWebView::loadProgress() const {
  Q_D(const OxideQQuickWebView);

  return d->load_progress_;
}

OxideQQuickWebFrame* OxideQQuickWebView::rootFrame() const {
  Q_D(const OxideQQuickWebView);

  return adapterToQObject<OxideQQuickWebFrame>(d->rootFrame());
}

QQmlListProperty<OxideQQuickScriptMessageHandler>
OxideQQuickWebView::messageHandlers() {
  return QQmlListProperty<OxideQQuickScriptMessageHandler>(
      this, NULL,
      OxideQQuickWebViewPrivate::messageHandler_append,
      OxideQQuickWebViewPrivate::messageHandler_count,
      OxideQQuickWebViewPrivate::messageHandler_at,
      OxideQQuickWebViewPrivate::messageHandler_clear);
}

void OxideQQuickWebView::addMessageHandler(
    OxideQQuickScriptMessageHandler* handler) {
  Q_D(OxideQQuickWebView);

  if (!handler) {
    qWarning() << "Didn't specify a handler";
    return;
  }

  OxideQQuickScriptMessageHandlerPrivate* hd =
      OxideQQuickScriptMessageHandlerPrivate::get(handler);

  if (hd->isActive() && handler->parent() != this) {
    qWarning() << "MessageHandler can't be added to more than one message target";
    return;
  }

  if (d->message_handlers().contains(hd)) {
    d->message_handlers().removeOne(hd);
  }

  handler->setParent(this);
  d->message_handlers().append(hd);

  emit messageHandlersChanged();
}

void OxideQQuickWebView::removeMessageHandler(
    OxideQQuickScriptMessageHandler* handler) {
  Q_D(OxideQQuickWebView);

  if (!handler) {
    qWarning() << "Didn't specify a handler";
    return;
  }

  OxideQQuickScriptMessageHandlerPrivate* hd =
      OxideQQuickScriptMessageHandlerPrivate::get(handler);

  if (!d->message_handlers().contains(hd)) {
    return;
  }

  handler->setParent(NULL);
  d->message_handlers().removeOne(hd);

  emit messageHandlersChanged();
}

QQmlComponent* OxideQQuickWebView::popupMenu() const {
  Q_D(const OxideQQuickWebView);

  return d->popup_menu_;
}

void OxideQQuickWebView::setPopupMenu(QQmlComponent* popup_menu) {
  Q_D(OxideQQuickWebView);

  if (d->popup_menu_ == popup_menu) {
    return;
  }

  d->popup_menu_ = popup_menu;
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

  OxideQQuickWebContext* c =
      adapterToQObject<OxideQQuickWebContext>(d->context());
  if (c == OxideQQuickWebContext::defaultContext(false)) {
    return NULL;
  }

  return c;
}

void OxideQQuickWebView::setContext(OxideQQuickWebContext* context) {
  Q_D(OxideQQuickWebView);

  if (d->isInitialized()) {
    qWarning() << "WebView context must be set during construction";
    return;
  }

  if (context == OxideQQuickWebContext::defaultContext(false)) {
    qWarning() <<
        "Setting WebView context to default context is unnecessary. WebView "
        "will automatically use the default context if it is created without "
        "one";
    return;
  }

  oxide::qt::WebContextAdapter* old = d->context();
  if (context == adapterToQObject<OxideQQuickWebContext>(old)) {
    return;
  }
  d->detachContextSignals(static_cast<OxideQQuickWebContextPrivate *>(old));

  OxideQQuickWebContextPrivate* cd = NULL;
  if (context) {
    cd = OxideQQuickWebContextPrivate::get(context);
  }
  d->attachContextSignals(cd);
  d->setContext(cd);

  emit contextChanged();
}

OxideQWebPreferences* OxideQQuickWebView::preferences() {
  Q_D(OxideQQuickWebView);

  return d->preferences();
}

void OxideQQuickWebView::setPreferences(OxideQWebPreferences* prefs) {
  Q_D(OxideQQuickWebView);

  if (prefs == d->preferences()) {
    return;
  }

  d->setPreferences(prefs);

  emit preferencesChanged();
}

OxideQQuickNavigationHistory* OxideQQuickWebView::navigationHistory() {
  Q_D(OxideQQuickWebView);

  return &d->navigation_history_;
}

// This exists purely to remove a moc warning. We don't store this request
// anywhere, it's only a transient object and I can't think of any possible
// reason why anybody would want to read it back
OxideQNewViewRequest* OxideQQuickWebView::request() const {
  return NULL;
}

void OxideQQuickWebView::setRequest(OxideQNewViewRequest* request) {
  Q_D(OxideQQuickWebView);

  d->setRequest(request);
}

// static
OxideQQuickWebViewAttached* OxideQQuickWebView::qmlAttachedProperties(
    QObject* object) {
  return new OxideQQuickWebViewAttached(object);
}

void OxideQQuickWebView::goBack() {
  Q_D(OxideQQuickWebView);

  d->goBack();
}

void OxideQQuickWebView::goForward() {
  Q_D(OxideQQuickWebView);

  d->goForward();
}

void OxideQQuickWebView::stop() {
  Q_D(OxideQQuickWebView);

  d->stop();
}

void OxideQQuickWebView::reload() {
  Q_D(OxideQQuickWebView);

  d->reload();
}

void OxideQQuickWebView::loadHtml(const QString& html, const QUrl& baseUrl) {
  Q_D(OxideQQuickWebView);

  d->loadHtml(html, baseUrl);
}

#include "moc_oxideqquickwebview_p.cpp"
