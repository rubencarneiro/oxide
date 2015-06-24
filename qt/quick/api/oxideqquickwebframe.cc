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

#include "oxideqquickwebframe_p.h"
#include "oxideqquickwebframe_p_p.h"

#include <QtDebug>
#include <QString>

#include "oxideqquickscriptmessagehandler_p.h"
#include "oxideqquickscriptmessagehandler_p_p.h"
#include "oxideqquickscriptmessagerequest_p.h"
#include "oxideqquickscriptmessagerequest_p_p.h"

OXIDE_Q_IMPL_PROXY_HANDLE_CONVERTER(OxideQQuickWebFrame,
                                    oxide::qt::WebFrameProxyHandle);

OxideQQuickWebFramePrivate::OxideQQuickWebFramePrivate(
    oxide::qt::WebFrameProxy* proxy,
    OxideQQuickWebFrame* q)
    : oxide::qt::WebFrameProxyHandle(proxy, q) {
  proxy->setClient(this);
}

// static
int OxideQQuickWebFramePrivate::childFrame_count(
    QQmlListProperty<OxideQQuickWebFrame>* prop) {
  oxide::qt::WebFrameProxy* p = OxideQQuickWebFramePrivate::get(
      static_cast<OxideQQuickWebFrame*>(prop->object))->proxy();

  return p->childFrameCount();
}

// static
OxideQQuickWebFrame* OxideQQuickWebFramePrivate::childFrame_at(
    QQmlListProperty<OxideQQuickWebFrame>* prop,
    int index) {
  oxide::qt::WebFrameProxy* p = OxideQQuickWebFramePrivate::get(
      static_cast<OxideQQuickWebFrame*>(prop->object))->proxy();

  return fromProxyHandle(p->childFrameAt(index));
}

// static
int OxideQQuickWebFramePrivate::messageHandler_count(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop) {
  oxide::qt::WebFrameProxy* p = OxideQQuickWebFramePrivate::get(
      static_cast<OxideQQuickWebFrame*>(prop->object))->proxy();

  return p->messageHandlers().size();
}

// static
OxideQQuickScriptMessageHandler* OxideQQuickWebFramePrivate::messageHandler_at(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop,
    int index) {
  oxide::qt::WebFrameProxy* p = OxideQQuickWebFramePrivate::get(
      static_cast<OxideQQuickWebFrame*>(prop->object))->proxy();

  return OxideQQuickScriptMessageHandlerPrivate::fromProxyHandle(
      p->messageHandlers().at(index));
}

void OxideQQuickWebFramePrivate::URLCommitted() {
  Q_Q(OxideQQuickWebFrame);

  emit q->urlChanged();
}

void OxideQQuickWebFramePrivate::ChildFramesChanged() {
  Q_Q(OxideQQuickWebFrame);

  emit q->childFramesChanged();
}

void OxideQQuickWebFramePrivate::DestroyFrame() {
  Q_Q(OxideQQuickWebFrame);

  delete q;
  // |this| has been destroyed
}

// static
OxideQQuickWebFrame* OxideQQuickWebFramePrivate::create(
    oxide::qt::WebFrameProxy* proxy) {
  OxideQQuickWebFrame* frame = new OxideQQuickWebFrame();
  frame->d_ptr.reset(new OxideQQuickWebFramePrivate(proxy, frame));
  return frame;
}

// static
OxideQQuickWebFramePrivate* OxideQQuickWebFramePrivate::get(
    OxideQQuickWebFrame* frame) {
  return frame->d_func();
}

OxideQQuickWebFrame::OxideQQuickWebFrame() {}

OxideQQuickWebFrame::~OxideQQuickWebFrame() {
  Q_D(OxideQQuickWebFrame);

  while (d->proxy()->messageHandlers().size() > 0) {
    delete OxideQQuickScriptMessageHandlerPrivate::fromProxyHandle(
        d->proxy()->messageHandlers().at(0));
  }
}

QUrl OxideQQuickWebFrame::url() const {
  Q_D(const OxideQQuickWebFrame);

  return d->proxy()->url();
}

OxideQQuickWebFrame* OxideQQuickWebFrame::parentFrame() const {
  Q_D(const OxideQQuickWebFrame);

  oxide::qt::WebFrameProxyHandle* p = d->proxy()->parent();
  if (!p) {
    return nullptr;
  }

  return OxideQQuickWebFramePrivate::fromProxyHandle(p);
}

QQmlListProperty<OxideQQuickWebFrame> OxideQQuickWebFrame::childFrames() {
  return QQmlListProperty<OxideQQuickWebFrame>(
      this, nullptr,
      OxideQQuickWebFramePrivate::childFrame_count,
      OxideQQuickWebFramePrivate::childFrame_at);
}

QQmlListProperty<OxideQQuickScriptMessageHandler>
OxideQQuickWebFrame::messageHandlers() {
  return QQmlListProperty<OxideQQuickScriptMessageHandler>(
      this, nullptr,
      OxideQQuickWebFramePrivate::messageHandler_count,
      OxideQQuickWebFramePrivate::messageHandler_at);
}

void OxideQQuickWebFrame::addMessageHandler(
    OxideQQuickScriptMessageHandler* handler) {
  Q_D(OxideQQuickWebFrame);

  if (!handler) {
    qWarning() << "OxideQQuickWebFrame::addMessageHandler: NULL handler";
    return;
  }

  OxideQQuickScriptMessageHandlerPrivate* hd =
      OxideQQuickScriptMessageHandlerPrivate::get(handler);

  if (hd->isActive() && handler->parent() != this) {
    qWarning() <<
        "OxideQQuickWebFrame::addMessageHandler: handler can't be added to "
        "more than one message target";
    return;
  }

  if (d->proxy()->messageHandlers().contains(hd)) {
    d->proxy()->messageHandlers().removeOne(hd);
  }

  handler->setParent(this);
  d->proxy()->messageHandlers().append(hd);

  emit messageHandlersChanged();
}

void OxideQQuickWebFrame::removeMessageHandler(
    OxideQQuickScriptMessageHandler* handler) {
  Q_D(OxideQQuickWebFrame);

  if (!handler) {
    qWarning() << "OxideQQuickWebFrame::addMessageHandler: NULL handler";
    return;
  }

  OxideQQuickScriptMessageHandlerPrivate* hd =
      OxideQQuickScriptMessageHandlerPrivate::get(handler);

  if (!d->proxy()->messageHandlers().contains(hd)) {
    return;
  }

  handler->setParent(nullptr);
  d->proxy()->messageHandlers().removeOne(hd);

  emit messageHandlersChanged();
}

OxideQQuickScriptMessageRequest* OxideQQuickWebFrame::sendMessage(
    const QUrl& context,
    const QString& msg_id,
    const QVariant& payload) {
  Q_D(OxideQQuickWebFrame);

  OxideQQuickScriptMessageRequest* request =
      new OxideQQuickScriptMessageRequest();

  QVariant aux = payload;
  if (aux.userType() == qMetaTypeId<QJSValue>()) {
    aux = aux.value<QJSValue>().toVariant();
  }

  if (!d->proxy()->sendMessage(
          context, msg_id, aux,
          OxideQQuickScriptMessageRequestPrivate::get(request))) {
    delete request;
    return nullptr;
  }

  return request;
}

void OxideQQuickWebFrame::sendMessageNoReply(const QUrl& context,
                                             const QString& msg_id,
                                             const QVariant& payload) {
  Q_D(OxideQQuickWebFrame);

  QVariant aux = payload;
  if (aux.userType() == qMetaTypeId<QJSValue>()) {
    aux = aux.value<QJSValue>().toVariant();
  }

  d->proxy()->sendMessageNoReply(context, msg_id, aux);
}
