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
#include <QQuickWindow>
#include <QRect>
#include <QRectF>
#include <QSizeF>
#include <QSize>
#include <QtQml>

#include "qt/core/api/oxideqloadevent.h"

#include "qt/quick/oxide_qquick_render_view_item.h"
#include "qt/quick/oxide_qquick_web_popup_menu_delegate.h"

#include "oxideqquickmessagehandler_p.h"
#include "oxideqquickmessagehandler_p_p.h"
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
    context(NULL),
    popup_menu(NULL),
    init_props_(new InitData()),
    q_ptr(view) {}

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

void OxideQQuickWebViewPrivate::URLChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->urlChanged();
}

void OxideQQuickWebViewPrivate::TitleChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->titleChanged();
}

void OxideQQuickWebViewPrivate::CommandsUpdated() {
  Q_Q(OxideQQuickWebView);

  emit q->navigationHistoryChanged();
}

void OxideQQuickWebViewPrivate::RootFrameChanged() {
  Q_Q(OxideQQuickWebView);

  // Make the webview the QObject parent of the new root frame,
  // to stop Qml from collecting the frame tree
  OxideQQuickWebFrame* root = q->rootFrame();
  if (root) {
    root->setParent(q);
  }

  emit q->rootFrameChanged();
}

void OxideQQuickWebViewPrivate::LoadStarted(const QUrl& url) {
  Q_Q(OxideQQuickWebView);

  OxideQLoadEvent event(url, OxideQLoadEvent::TypeStarted);
  emit q->loadingChanged(&event);
}

void OxideQQuickWebViewPrivate::LoadStopped(const QUrl& url) {
  Q_Q(OxideQQuickWebView);

  OxideQLoadEvent event(url, OxideQLoadEvent::TypeStopped);
  emit q->loadingChanged(&event);
}

void OxideQQuickWebViewPrivate::LoadFailed(const QUrl& url,
                                           int error_code,
                                           const QString& error_description) {
  Q_Q(OxideQQuickWebView);

  OxideQLoadEvent event(url, OxideQLoadEvent::TypeFailed,
                        error_code, error_description);
  emit q->loadingChanged(&event);
}

void OxideQQuickWebViewPrivate::LoadSucceeded(const QUrl& url) {
  Q_Q(OxideQQuickWebView);

  OxideQLoadEvent event(url, OxideQLoadEvent::TypeSucceeded);
  emit q->loadingChanged(&event);
}

oxide::qt::WebFrameAdapter* OxideQQuickWebViewPrivate::CreateWebFrame() {
  return OxideQQuickWebFramePrivate::get(new OxideQQuickWebFrame());
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

void OxideQQuickWebViewPrivate::componentComplete() {
  Q_Q(OxideQQuickWebView);

  Q_ASSERT(init_props_);

  if (!context) {
    // Ok, we handle the default context a bit differently. If our context
    // comes from setContext(), then we don't hold a strong reference to it
    // because it will be owned by someone else in the QML object
    // hierarchy. However, the default context is not in this hierarchy and
    // has no QObject parent, so we use reference counting for it instead to
    // ensure that it is freed once all webviews are closed
    default_context_.reset(OxideQQuickWebContext::defaultContext());
    context = default_context_.data();
  }

  init(OxideQQuickWebContextPrivate::get(context),
       QSizeF(q->width(), q->height()).toSize(),
       init_props_->incognito,
       init_props_->url,
       q->isVisible());

  init_props_.reset();
}

// static
void OxideQQuickWebViewPrivate::messageHandler_append(
    QQmlListProperty<OxideQQuickMessageHandler>* prop,
    OxideQQuickMessageHandler* message_handler) {
  if (!message_handler) {
    return;
  }

  OxideQQuickWebView* web_view =
      static_cast<OxideQQuickWebView* >(prop->object);

  web_view->addMessageHandler(message_handler);
}

// static
int OxideQQuickWebViewPrivate::messageHandler_count(
    QQmlListProperty<OxideQQuickMessageHandler>* prop) {
  OxideQQuickWebViewPrivate* p = OxideQQuickWebViewPrivate::get(
        static_cast<OxideQQuickWebView *>(prop->object));

  return p->message_handlers().size();
}

// static
OxideQQuickMessageHandler* OxideQQuickWebViewPrivate::messageHandler_at(
    QQmlListProperty<OxideQQuickMessageHandler>* prop,
    int index) {
  OxideQQuickWebViewPrivate* p = OxideQQuickWebViewPrivate::get(
        static_cast<OxideQQuickWebView *>(prop->object));

  if (index >= p->message_handlers().size()) {
    return NULL;
  }

  return adapterToQObject<OxideQQuickMessageHandler>(
      p->message_handlers().at(index));
}

// static
void OxideQQuickWebViewPrivate::messageHandler_clear(
    QQmlListProperty<OxideQQuickMessageHandler>* prop) {
  OxideQQuickWebView* web_view =
      static_cast<OxideQQuickWebView *>(prop->object);
  OxideQQuickWebViewPrivate* p = OxideQQuickWebViewPrivate::get(web_view);

  p->message_handlers().clear();

  emit web_view->messageHandlersChanged();
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
    QQuickItem(parent),
    d_ptr(new OxideQQuickWebViewPrivate(this)) {
  QObject::connect(this, SIGNAL(visibleChanged()),
                   this, SLOT(visibilityChangedListener()));
}

OxideQQuickWebView::~OxideQQuickWebView() {
  Q_D(OxideQQuickWebView);

  // This is a bit hacky, but when we are using the default context,
  // we hold a reference to it. We release this reference before
  // the oxide::WebView destructor is called, so we have to ensure our
  // WebContents is destroyed now
  d->shutdown();

  delete d_ptr;
  d_ptr = NULL;

  QObject::disconnect(this, SIGNAL(visibleChanged()),
                      this, SLOT(visibilityChangedListener()));
}

void OxideQQuickWebView::componentComplete() {
  Q_D(OxideQQuickWebView);

  QQuickItem::componentComplete();

  d->componentComplete();
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

OxideQQuickWebFrame* OxideQQuickWebView::rootFrame() const {
  Q_D(const OxideQQuickWebView);

  return adapterToQObject<OxideQQuickWebFrame>(d->rootFrame());
}

QQmlListProperty<OxideQQuickMessageHandler>
OxideQQuickWebView::messageHandlers() {
  return QQmlListProperty<OxideQQuickMessageHandler>(
      this, NULL,
      OxideQQuickWebViewPrivate::messageHandler_append,
      OxideQQuickWebViewPrivate::messageHandler_count,
      OxideQQuickWebViewPrivate::messageHandler_at,
      OxideQQuickWebViewPrivate::messageHandler_clear);
}

void OxideQQuickWebView::addMessageHandler(
    OxideQQuickMessageHandler* handler) {
  Q_D(OxideQQuickWebView);

  if (!handler) {
    qWarning() << "Didn't specify a handler";
    return;
  }

  OxideQQuickMessageHandlerPrivate* handlerp =
      OxideQQuickMessageHandlerPrivate::get(handler);

  if (!d->message_handlers().contains(handlerp)) {
    handlerp->removeFromCurrentOwner();
    handler->setParent(this);

    d->message_handlers().append(handlerp);

    emit messageHandlersChanged();
  }
}

void OxideQQuickWebView::removeMessageHandler(
    OxideQQuickMessageHandler* handler) {
  Q_D(OxideQQuickWebView);

  if (!handler) {
    qWarning() << "Didn't specify a handler";
    return;
  }

  if (!d) {
    return;
  }

  OxideQQuickMessageHandlerPrivate* handlerp =
      OxideQQuickMessageHandlerPrivate::get(handler);

  if (d->message_handlers().contains(handlerp)) {
    d->message_handlers().removeOne(handlerp);
    handler->setParent(NULL);

    emit messageHandlersChanged();
  }
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

  d->context = context;
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
