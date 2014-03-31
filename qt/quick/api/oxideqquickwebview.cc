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

#include <QPointF>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QRect>
#include <QRectF>
#include <QSizeF>
#include <QSize>
#include <QtQml>

#include "qt/quick/oxide_qquick_alert_dialog_delegate.h"
#include "qt/quick/oxide_qquick_before_unload_dialog_delegate.h"
#include "qt/quick/oxide_qquick_confirm_dialog_delegate.h"
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

void OxideQQuickWebViewPrivate::contextInitialized() {
  completeConstruction();
}

void OxideQQuickWebViewPrivate::contextWillBeDestroyed() {
  Q_Q(OxideQQuickWebView);

  // XXX: Our underlying BrowserContext lives on, so we're left in a
  // bit of a weird state here (WebView.context will return no context,
  // which is a lie)
  context = NULL;
  emit q->contextChanged();
}

void OxideQQuickWebViewPrivate::detachContextSignals() {
  Q_Q(OxideQQuickWebView);

  if (context) {
    QObject::disconnect(OxideQQuickWebContextPrivate::get(context),
                        SIGNAL(initialized()),
                        q, SLOT(contextInitialized()));
    QObject::disconnect(OxideQQuickWebContextPrivate::get(context),
                        SIGNAL(willBeDestroyed()),
                        q, SLOT(contextWillBeDestroyed()));
  }
}

OxideQQuickWebViewPrivate::OxideQQuickWebViewPrivate(
    OxideQQuickWebView* view) :
    oxide::qt::WebViewAdapter(view),
    context(NULL),
    navigationHistory(view),
    popup_menu(NULL),
    alert_dialog(NULL),
    confirm_dialog(NULL),
    prompt_dialog(NULL),
    before_unload_dialog(NULL),
    init_props_(new InitData()),
    load_progress_(0) {}

OxideQQuickWebViewPrivate::~OxideQQuickWebViewPrivate() {
}

oxide::qt::RenderWidgetHostViewDelegate*
OxideQQuickWebViewPrivate::CreateRenderWidgetHostViewDelegate() {
  Q_Q(OxideQQuickWebView);

  return new oxide::qquick::RenderViewItem(q);
}

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
  navigationHistory.onNavigationEntryCommitted();
}

void OxideQQuickWebViewPrivate::NavigationListPruned(bool from_front, int count) {
  navigationHistory.onNavigationListPruned(from_front, count);
}

void OxideQQuickWebViewPrivate::NavigationEntryChanged(int index) {
  navigationHistory.onNavigationEntryChanged(index);
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

void OxideQQuickWebViewPrivate::completeConstruction() {
  Q_Q(OxideQQuickWebView);

  Q_ASSERT(init_props_);

  OxideQQuickWebContext* context_in_use = context;
  if (!context_in_use) {
    // The default context is reference counted and not exposed to the
    // embedder
    default_context_ = OxideQQuickWebContext::defaultContext();
    context_in_use = default_context_.data();
  }

  init(OxideQQuickWebContextPrivate::get(context_in_use),
       QSizeF(q->width(), q->height()).toSize(),
       init_props_->incognito,
       init_props_->url,
       q->isVisible());

  init_props_.reset();

  // Make the webview the QObject parent of the new root frame,
  // to stop Qml from collecting the frame tree
  q->rootFrame()->setParent(q);

  // Initialization created the root frame. This is the only time
  // this is emitted
  emit q->rootFrameChanged();
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

void OxideQQuickWebView::visibilityChangedListener() {
  Q_D(OxideQQuickWebView);

  if (!d->isInitialized()) {
    return;
  }

  d->updateVisibility(isVisible());
}

void OxideQQuickWebView::geometryChanged(const QRectF& newGeometry,
                                         const QRectF& oldGeometry) {
  Q_D(OxideQQuickWebView);

  QQuickItem::geometryChanged(newGeometry, oldGeometry);

  if (d->isInitialized()) {
    d->updateSize(newGeometry.size().toSize());
  }
}

OxideQQuickWebView::OxideQQuickWebView(QQuickItem* parent) :
    QQuickItem(parent) {
  // WebView instantiates NotificationRegistrar, which starts
  // NotificationService, which uses LazyInstance. Start Chromium now
  // else we'll crash
  OxideQQuickWebContextPrivate::ensureChromiumStarted();
  d_ptr.reset(new OxideQQuickWebViewPrivate(this));
  QObject::connect(this, SIGNAL(visibleChanged()),
                   this, SLOT(visibilityChangedListener()));
}

OxideQQuickWebView::~OxideQQuickWebView() {
  Q_D(OxideQQuickWebView);

  disconnect(this, SIGNAL(visibleChanged()),
             this, SLOT(visibilityChangedListener()));
  d->detachContextSignals();

  // Do this before our d_ptr is cleared, as these call back in to us
  // when they are deleted
  while (d->message_handlers().size() > 0) {
    delete adapterToQObject<OxideQQuickScriptMessageHandler>(
        d->message_handlers().at(0));
  }
}

void OxideQQuickWebView::componentComplete() {
  Q_D(OxideQQuickWebView);

  QQuickItem::componentComplete();

  if (d->context) {
    connect(OxideQQuickWebContextPrivate::get(d->context),
            SIGNAL(initialized()),
            this, SLOT(contextInitialized()));
  }

  if (!d->context ||
      OxideQQuickWebContextPrivate::get(d->context)->isInitialized()) {
    d->completeConstruction();
  }
}

QUrl OxideQQuickWebView::url() const {
  Q_D(const OxideQQuickWebView);

  return d->url();
}

void OxideQQuickWebView::setUrl(const QUrl& url) {
  Q_D(OxideQQuickWebView);

  if (d->init_props()) {
    d->init_props()->url = url;
  } else {
    d->setUrl(url);
  }
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

  if (d->isInitialized()) {
    qWarning() << "Cannot change incognito mode after initialization";
    return;
  }

  d->init_props()->incognito = incognito;
}

bool OxideQQuickWebView::loading() const {
  Q_D(const OxideQQuickWebView);

  return d->loading();
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

  return d->popup_menu;
}

void OxideQQuickWebView::setPopupMenu(QQmlComponent* popup_menu) {
  Q_D(OxideQQuickWebView);

  if (d->popup_menu == popup_menu) {
    return;
  }

  d->popup_menu = popup_menu;
  emit popupMenuChanged();
}

QQmlComponent* OxideQQuickWebView::alertDialog() const {
  Q_D(const OxideQQuickWebView);

  return d->alert_dialog;
}

void OxideQQuickWebView::setAlertDialog(QQmlComponent* alert_dialog) {
  Q_D(OxideQQuickWebView);

  if (d->alert_dialog == alert_dialog) {
    return;
  }

  d->alert_dialog = alert_dialog;
  emit alertDialogChanged();
}

QQmlComponent* OxideQQuickWebView::confirmDialog() const {
  Q_D(const OxideQQuickWebView);

  return d->confirm_dialog;
}

void OxideQQuickWebView::setConfirmDialog(QQmlComponent* confirm_dialog) {
  Q_D(OxideQQuickWebView);

  if (d->confirm_dialog == confirm_dialog) {
    return;
  }

  d->confirm_dialog = confirm_dialog;
  emit confirmDialogChanged();
}

QQmlComponent* OxideQQuickWebView::promptDialog() const {
  Q_D(const OxideQQuickWebView);

  return d->prompt_dialog;
}

void OxideQQuickWebView::setPromptDialog(QQmlComponent* prompt_dialog) {
  Q_D(OxideQQuickWebView);

  if (d->prompt_dialog == prompt_dialog) {
    return;
  }

  d->prompt_dialog = prompt_dialog;
  emit promptDialogChanged();
}

QQmlComponent* OxideQQuickWebView::beforeUnloadDialog() const {
  Q_D(const OxideQQuickWebView);

  return d->before_unload_dialog;
}

void OxideQQuickWebView::setBeforeUnloadDialog(
    QQmlComponent* before_unload_dialog) {
  Q_D(OxideQQuickWebView);

  if (d->before_unload_dialog == before_unload_dialog) {
    return;
  }

  d->before_unload_dialog = before_unload_dialog;
  emit beforeUnloadDialogChanged();
}

OxideQQuickWebContext* OxideQQuickWebView::context() const {
  Q_D(const OxideQQuickWebView);

  return d->context;
}

void OxideQQuickWebView::setContext(OxideQQuickWebContext* context) {
  Q_D(OxideQQuickWebView);

  if (!d->init_props()) {
    qWarning() << "The context can only be set at construction time";
    return;
  }

  if (context == d->context) {
    return;
  }

  d->detachContextSignals();
  if (context) {
    connect(OxideQQuickWebContextPrivate::get(context), SIGNAL(willBeDestroyed()),
            this, SLOT(contextWillBeDestroyed()));
  }

  d->context = context;
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
  // We don't emit a signal here, as we get OnWebPreferencesChanged(),
  // which also happens if our WebPreferences are destroyed
}

OxideQQuickNavigationHistory* OxideQQuickWebView::navigationHistory() {
  Q_D(OxideQQuickWebView);

  return &d->navigationHistory;
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

#include "moc_oxideqquickwebview_p.cpp"
