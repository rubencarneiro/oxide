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

#include "oxide_qquick_web_view_p.h"

#include <QRectF>

#include "qt/core/api/private/oxide_qquick_web_view_p_p.h"
#include "qt/core/api/private/oxide_qt_qmessage_handler_p.h"
#include "qt/core/browser/oxide_qt_web_frame.h"

#include "oxide_qquick_message_handler_p.h"

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

void OxideQQuickWebView::visibilityChangedListener() {
  Q_D(oxide::qt::QQuickWebView);

  d->UpdateVisibility();
}

void OxideQQuickWebView::geometryChanged(const QRectF& newGeometry,
                                         const QRectF& oldGeometry) {
  Q_D(oxide::qt::QQuickWebView);

  QQuickItem::geometryChanged(newGeometry, oldGeometry);

  for (int i = 0; i < childItems().count(); ++i) {
    QQuickItem* item = childItems().at(i);
    item->setSize(newGeometry.size());
  }

  if (d->web_contents()) {
    d->updateSize(newGeometry.size());
  }
}

OxideQQuickWebView::OxideQQuickWebView(QQuickItem* parent) :
    QQuickItem(parent),
    d_ptr(oxide::qt::QQuickWebViewPrivate::Create(this)) {
  QObject::connect(this, SIGNAL(visibleChanged()),
                   this, SLOT(visibilityChangedListener()));
}

OxideQQuickWebView::~OxideQQuickWebView() {
  QObject::disconnect(this, SIGNAL(visibleChanged()),
                      this, SLOT(visibilityChangedListener()));
}

void OxideQQuickWebView::componentComplete() {
  Q_D(oxide::qt::QQuickWebView);

  QQuickItem::componentComplete();

  d->componentComplete();
}

QUrl OxideQQuickWebView::url() const {
  Q_D(const oxide::qt::QQuickWebView);

  return d->url();
}

void OxideQQuickWebView::setUrl(const QUrl& url) {
  Q_D(oxide::qt::QQuickWebView);

  if (d->init_props()) {
    d->init_props()->url = url;
  } else {
    d->setUrl(url);
  }
}

QString OxideQQuickWebView::title() const {
  Q_D(const oxide::qt::QQuickWebView);

  return QString::fromStdString(d->GetTitle());
}

bool OxideQQuickWebView::canGoBack() const {
  Q_D(const oxide::qt::QQuickWebView);

  return d->CanGoBack();
}

bool OxideQQuickWebView::canGoForward() const {
  Q_D(const oxide::qt::QQuickWebView);

  return d->CanGoForward();
}

bool OxideQQuickWebView::incognito() const {
  Q_D(const oxide::qt::QQuickWebView);

  return d->IsIncognito();
}

void OxideQQuickWebView::setIncognito(bool incognito) {
  Q_D(oxide::qt::QQuickWebView);

  if (d->init_props()) {
    d->init_props()->incognito = incognito;
  }
}

bool OxideQQuickWebView::loading() const {
  Q_D(const oxide::qt::QQuickWebView);

  return d->IsLoading();
}

OxideQQuickWebFrame* OxideQQuickWebView::rootFrame() const {
  Q_D(const oxide::qt::QQuickWebView);

  oxide::qt::WebFrameQQuick* root =
      static_cast<oxide::qt::WebFrameQQuick *>(d->GetRootFrame());
  if (!root) {
    return NULL;
  }

  return root->QQuickWebFrame();
}

QQmlListProperty<OxideQQuickMessageHandler>
OxideQQuickWebView::messageHandlers() {
  return QQmlListProperty<OxideQQuickMessageHandler>(
      this, NULL,
      oxide::qt::QQuickWebViewPrivate::messageHandler_append,
      oxide::qt::QQuickWebViewPrivate::messageHandler_count,
      oxide::qt::QQuickWebViewPrivate::messageHandler_at,
      oxide::qt::QQuickWebViewPrivate::messageHandler_clear);
}

void OxideQQuickWebView::addMessageHandler(
    OxideQQuickMessageHandler* handler) {
  Q_D(oxide::qt::QQuickWebView);

  if (!d->message_handlers().contains(handler)) {
    oxide::qt::QQuickMessageHandlerPrivate::get(handler)->removeFromCurrentOwner();
    handler->setParent(this);

    d->message_handlers().append(handler);

    emit messageHandlersChanged();
  }
}

void OxideQQuickWebView::removeMessageHandler(
    OxideQQuickMessageHandler* handler) {
  Q_D(oxide::qt::QQuickWebView);

  if (!d) {
    return;
  }

  if (d->message_handlers().contains(handler)) {
    d->message_handlers().removeOne(handler);
    handler->setParent(NULL);

    emit messageHandlersChanged();
  }
}

QQmlComponent* OxideQQuickWebView::popupMenu() const {
  Q_D(const oxide::qt::QQuickWebView);

  return d->popup_menu;
}

void OxideQQuickWebView::setPopupMenu(QQmlComponent* popup_menu) {
  Q_D(oxide::qt::QQuickWebView);

  d->popup_menu = popup_menu;
  emit popupMenuChanged();
}

OxideQQuickWebContext* OxideQQuickWebView::context() const {
  Q_D(const oxide::qt::QQuickWebView);

  return d->context;
}

void OxideQQuickWebView::setContext(OxideQQuickWebContext* context) {
  Q_D(oxide::qt::QQuickWebView);

  if (!d->web_contents()) {
    Q_ASSERT(!d->context);
    d->context = context;
  }
}

// static
OxideQQuickWebViewAttached* OxideQQuickWebView::qmlAttachedProperties(
    QObject* object) {
  return new OxideQQuickWebViewAttached(object);
}

void OxideQQuickWebView::goBack() {
  Q_D(oxide::qt::QQuickWebView);

  d->GoBack();
}

void OxideQQuickWebView::goForward() {
  Q_D(oxide::qt::QQuickWebView);

  d->GoForward();
}

void OxideQQuickWebView::stop() {
  Q_D(oxide::qt::QQuickWebView);

  d->Stop();
}

void OxideQQuickWebView::reload() {
  Q_D(oxide::qt::QQuickWebView);

  d->Reload();
}
