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

#include "oxide_qquick_web_view_p_p.h"

#include <QPointF>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSizeF>
#include <QString>

#include "ui/gfx/size.h"
#include "url/gurl.h"

#include "qt/lib/api/public/oxide_q_load_status.h"
#include "qt/lib/api/public/oxide_qquick_web_view_p.h"
#include "qt/lib/api/public/oxide_qquick_web_view_context_p.h"
#include "qt/lib/browser/oxide_qt_render_widget_host_view_qquick.h"
#include "qt/lib/browser/oxide_qt_web_frame.h"
#include "qt/lib/browser/oxide_qt_web_popup_menu_qquick.h"

#include "oxide_qt_qmessage_handler_p.h"
#include "oxide_qt_qweb_view_context_p.h"

namespace oxide {
namespace qt {

QQuickWebViewPrivate::QQuickWebViewPrivate(OxideQQuickWebView* view) :
    context_(NULL),
    popup_menu_(NULL),
    q_ptr(view),
    init_props_(new InitData()),
    weak_factory_(this) {}

void QQuickWebViewPrivate::OnURLChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->urlChanged();
}

void QQuickWebViewPrivate::OnTitleChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->titleChanged();
}

void QQuickWebViewPrivate::OnCommandsUpdated() {
  Q_Q(OxideQQuickWebView);

  emit q->navigationHistoryChanged();
}

void QQuickWebViewPrivate::OnRootFrameChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->rootFrameChanged();
}

void QQuickWebViewPrivate::OnLoadStarted(const GURL& validated_url,
                                         bool is_error_frame) {
  Q_Q(OxideQQuickWebView);

  OxideQLoadStatus* status =
      new OxideQLoadStatus(QUrl(QString::fromStdString(validated_url.spec())),
                           OxideQLoadStatus::LoadStatusStarted);
  QQmlEngine::setObjectOwnership(status, QQmlEngine::JavaScriptOwnership);

  emit q->loadingChanged(status);
}

void QQuickWebViewPrivate::OnLoadStopped(const GURL& validated_url) {
  Q_Q(OxideQQuickWebView);

  OxideQLoadStatus* status =
      new OxideQLoadStatus(QUrl(QString::fromStdString(validated_url.spec())),
                           OxideQLoadStatus::LoadStatusStopped);
  QQmlEngine::setObjectOwnership(status, QQmlEngine::JavaScriptOwnership);

  emit q->loadingChanged(status);
}

void QQuickWebViewPrivate::OnLoadFailed(
    const GURL& validated_url,
    int error_code,
    const std::string& error_description) {
  Q_Q(OxideQQuickWebView);

  OxideQLoadStatus* status =
      new OxideQLoadStatus(QUrl(QString::fromStdString(validated_url.spec())),
                           OxideQLoadStatus::LoadStatusFailed,
                           error_code,
                           QString::fromStdString(error_description));
  QQmlEngine::setObjectOwnership(status, QQmlEngine::JavaScriptOwnership);

  emit q->loadingChanged(status);
}

void QQuickWebViewPrivate::OnLoadSucceeded(const GURL& validated_url) {
  Q_Q(OxideQQuickWebView);

  OxideQLoadStatus* status =
      new OxideQLoadStatus(QUrl(QString::fromStdString(validated_url.spec())),
                           OxideQLoadStatus::LoadStatusSucceeded);
  QQmlEngine::setObjectOwnership(status, QQmlEngine::JavaScriptOwnership);

  emit q->loadingChanged(status);
}

oxide::WebFrame* QQuickWebViewPrivate::AllocWebFrame(
    int64 frame_id) {
  return new WebFrameQQuick(frame_id);
}

// static
QQuickWebViewPrivate* QQuickWebViewPrivate::Create(OxideQQuickWebView* view) {
  return new QQuickWebViewPrivate(view);
}

QQuickWebViewPrivate::~QQuickWebViewPrivate() {
  // It's important that this goes away before our context
  DestroyWebContents();
}

oxide::MessageDispatcherBrowser::MessageHandlerVector
QQuickWebViewPrivate::GetMessageHandlers() const {
  oxide::MessageDispatcherBrowser::MessageHandlerVector list;
  for (int i = 0; i < message_handlers_.size(); ++i) {
    list.push_back(QMessageHandlerBasePrivate::get(
        message_handlers_.at(i))->handler());
  }

  return list;
}

void QQuickWebViewPrivate::UpdateVisibility() {
  Q_Q(OxideQQuickWebView);

  if (init_props_) {
    return;
  }

  if (q->isVisible()) {
    Shown();
  } else {
    Hidden();
  }  
}

content::RenderWidgetHostView* QQuickWebViewPrivate::CreateViewForWidget(
    content::RenderWidgetHost* render_widget_host) {
  Q_Q(OxideQQuickWebView);

  return new RenderWidgetHostViewQQuick(render_widget_host, q);
}

gfx::Rect QQuickWebViewPrivate::GetContainerBounds() {
  Q_Q(OxideQQuickWebView);

  QPointF pos(q->mapToScene(QPointF(0,0)));
  if (q->window()) {
    // We could be called before being added to a scene
    pos += q->window()->position();
  }

  return gfx::Rect(qRound(pos.x()),
                   qRound(pos.y()),
                   qRound(q->width()),
                   qRound(q->height()));
}

oxide::WebPopupMenu* QQuickWebViewPrivate::CreatePopupMenu() {
  Q_Q(OxideQQuickWebView);

  return new WebPopupMenuQQuick(q, web_contents());
}

void QQuickWebViewPrivate::componentComplete() {
  Q_Q(OxideQQuickWebView);

  Q_ASSERT(init_props_);

  if (!context_) {
    // Ok, we handle the default context a bit differently. If our context
    // comes from setContext(), then we don't hold a strong reference to it
    // because it will be owned by someone else in the QML object
    // hierarchy. However, the default context is not in this hierarchy and
    // has no QObject parent, so we use reference counting for it instead to
    // ensure that it is freed once all webviews are closed
    default_context_.reset(OxideQQuickWebViewContext::defaultContext());
    context_ = default_context_.data();
  }

  Init(QWebViewContextBasePrivate::get(context_)->GetContext(),
       this, init_props_->incognito,
       gfx::Size(qRound(q->width()), qRound(q->height())));

  if (!init_props_->url.isEmpty()) {
    SetURL(GURL(init_props_->url.toString().toStdString()));
  }

  init_props_.reset();

  UpdateVisibility();
}

// static
void QQuickWebViewPrivate::messageHandler_append(
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
int QQuickWebViewPrivate::messageHandler_count(
    QQmlListProperty<OxideQQuickMessageHandler>* prop) {
  QQuickWebViewPrivate* p = QQuickWebViewPrivate::get(
        static_cast<OxideQQuickWebView *>(prop->object));

  return p->message_handlers().size();
}

// static
OxideQQuickMessageHandler* QQuickWebViewPrivate::messageHandler_at(
    QQmlListProperty<OxideQQuickMessageHandler>* prop,
    int index) {
  QQuickWebViewPrivate* p = QQuickWebViewPrivate::get(
        static_cast<OxideQQuickWebView *>(prop->object));

  if (index >= p->message_handlers().size()) {
    return NULL;
  }

  return p->message_handlers().at(index);
}

// static
void QQuickWebViewPrivate::messageHandler_clear(
    QQmlListProperty<OxideQQuickMessageHandler>* prop) {
  OxideQQuickWebView* web_view =
      static_cast<OxideQQuickWebView *>(prop->object);
  QQuickWebViewPrivate* p = QQuickWebViewPrivate::get(web_view);

  p->message_handlers().clear();

  emit web_view->messageHandlersChanged();
}

// static
QQuickWebViewPrivate* QQuickWebViewPrivate::get(OxideQQuickWebView* view) {
  return view->d_func();
}

void QQuickWebViewPrivate::updateSize(const QSizeF& size) {
  UpdateSize(gfx::Size(qRound(size.width()), qRound(size.height())));
}

QUrl QQuickWebViewPrivate::url() const {
  return QUrl(QString::fromStdString(GetURL().spec()));
}

void QQuickWebViewPrivate::setUrl(const QUrl& url) {
  SetURL(GURL(url.toString().toStdString()));
}

} // namespace qt
} // namespace oxide
