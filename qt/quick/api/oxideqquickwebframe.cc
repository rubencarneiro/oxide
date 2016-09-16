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

#include "oxideqquickwebframe.h"
#include "oxideqquickwebframe_p.h"

#include <QtDebug>
#include <QString>

#include "qt/core/glue/oxide_qt_web_frame_proxy.h"

#include "oxideqquickscriptmessagehandler.h"
#include "oxideqquickscriptmessagehandler_p.h"
#include "oxideqquickscriptmessagerequest.h"
#include "oxideqquickscriptmessagerequest_p.h"

OxideQQuickWebFramePrivate::OxideQQuickWebFramePrivate(
    oxide::qt::WebFrameProxy* proxy,
    OxideQQuickWebFrame* q)
    : q_ptr(q),
      proxy_(proxy) {
  proxy->setClient(this);
  proxy->setHandle(q);
}

// static
int OxideQQuickWebFramePrivate::childFrame_count(
    QQmlListProperty<OxideQQuickWebFrame>* prop) {
  oxide::qt::WebFrameProxy* p = OxideQQuickWebFramePrivate::get(
      static_cast<OxideQQuickWebFrame*>(prop->object))->proxy_.data();

  return p->childFrames().size();
}

// static
OxideQQuickWebFrame* OxideQQuickWebFramePrivate::childFrame_at(
    QQmlListProperty<OxideQQuickWebFrame>* prop,
    int index) {
  oxide::qt::WebFrameProxy* p = OxideQQuickWebFramePrivate::get(
      static_cast<OxideQQuickWebFrame*>(prop->object))->proxy_.data();

  return qobject_cast<OxideQQuickWebFrame*>(p->childFrames().at(index));
}

// static
int OxideQQuickWebFramePrivate::messageHandler_count(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop) {
  oxide::qt::WebFrameProxy* p = OxideQQuickWebFramePrivate::get(
      static_cast<OxideQQuickWebFrame*>(prop->object))->proxy_.data();

  return p->messageHandlers().size();
}

// static
OxideQQuickScriptMessageHandler* OxideQQuickWebFramePrivate::messageHandler_at(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop,
    int index) {
  oxide::qt::WebFrameProxy* p = OxideQQuickWebFramePrivate::get(
      static_cast<OxideQQuickWebFrame*>(prop->object))->proxy_.data();

  return qobject_cast<OxideQQuickScriptMessageHandler*>(
      p->messageHandlers().at(index));
}

void OxideQQuickWebFramePrivate::LoadCommitted() {
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

/*!
\class OxideQQuickWebFrame
\inmodule OxideQtQuick
\inheaderfile oxideqquickwebframe.h

\brief Represents a frame in a web page
*/

/*!
\qmltype WebFrame
\inqmlmodule com.canonical.Oxide 1.0
\instantiates OxideQQuickWebFrame

\brief Represents a frame in a web page

WebFrame represents a frame in a web page. It provides a way to access the
currently committed URL (via \l{url}) as well as a mechanism to communicate with
user scripts that the application has injected (via WebContext::userScripts).
*/

OxideQQuickWebFrame::OxideQQuickWebFrame() {}

/*!
\internal
*/

OxideQQuickWebFrame::~OxideQQuickWebFrame() {
  Q_D(OxideQQuickWebFrame);

  while (d->proxy_->messageHandlers().size() > 0) {
    delete d->proxy_->messageHandlers().at(0);
  }
}

/*!
\qmlproperty url WebFrame::url

The URL of the document that is currently displayed in this frame.
*/

QUrl OxideQQuickWebFrame::url() const {
  Q_D(const OxideQQuickWebFrame);

  return d->proxy_->url();
}

/*!
\qmlproperty WebFrame WebFrame::parentFrame

The WebFrame that this frame is a child of. This remains constant for the life
of the frame.
*/

OxideQQuickWebFrame* OxideQQuickWebFrame::parentFrame() const {
  Q_D(const OxideQQuickWebFrame);

  QObject* p = d->proxy_->parent();
  if (!p) {
    return nullptr;
  }

  return qobject_cast<OxideQQuickWebFrame*>(p);
}

/*!
\qmlproperty list<WebFrame> WebFrame::childFrames

A list of WebFrames that are children of this frame.
*/

QQmlListProperty<OxideQQuickWebFrame> OxideQQuickWebFrame::childFrames() {
  return QQmlListProperty<OxideQQuickWebFrame>(
      this, nullptr,
      OxideQQuickWebFramePrivate::childFrame_count,
      OxideQQuickWebFramePrivate::childFrame_at);
}

/*!
\qmlproperty list<ScriptMessageHandler> WebFrame::messageHandlers

The list of script message handlers attached to this frame. These can handle
messages from user scripts injected in to this frame.
*/

QQmlListProperty<OxideQQuickScriptMessageHandler>
OxideQQuickWebFrame::messageHandlers() {
  return QQmlListProperty<OxideQQuickScriptMessageHandler>(
      this, nullptr,
      OxideQQuickWebFramePrivate::messageHandler_count,
      OxideQQuickWebFramePrivate::messageHandler_at);
}

/*!
\qmlmethod void WebFrame::addMessageHandler(ScriptMessageHandler handler)

Add \a{handler} to the list of script message handlers attached to this frame.
If \a{handler} is already attached to another WebView or WebFrame, then this
will return without making any changes.

If \a{handler} is already attached to this frame then it will be moved to the
end of the list.

This frame will assume ownership of \a{handle} by setting itself as the
parent.

If \a{handle} is deleted whilst it is attached to this frame, then it will
automatically be detached.
*/

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

  if (d->proxy_->messageHandlers().contains(handler)) {
    d->proxy_->messageHandlers().removeOne(handler);
  }

  handler->setParent(this);
  d->proxy_->messageHandlers().append(handler);

  emit messageHandlersChanged();
}

/*!
\qmlmethod void WebFrame::removeMessageHandler(ScriptMessageHandler handler)

Remove \a{handler} from the list of script message handlers attached to this
frame.

This will unparent \a{handler}, giving ownership of it back to the application.
*/

void OxideQQuickWebFrame::removeMessageHandler(
    OxideQQuickScriptMessageHandler* handler) {
  Q_D(OxideQQuickWebFrame);

  if (!handler) {
    qWarning() << "OxideQQuickWebFrame::addMessageHandler: NULL handler";
    return;
  }

  if (!d->proxy_->messageHandlers().contains(handler)) {
    return;
  }

  handler->setParent(nullptr);
  d->proxy_->messageHandlers().removeOne(handler);

  emit messageHandlersChanged();
}

/*!
\qmlmethod ScriptMessageRequest WebFrame::sendMessage(url context,
                                                      string msgId,
                                                      variant payload)

Send a message to the handler for \a{msgId} provided by the user script injected
in to the JS context identified by \a{context}. The message payload is supplied
via \{payload}. \a{payload} must be a value, object or array that can be
represented by JSON values.

This returns a ScriptMessageRequest instance which is owned by the application
and can be used to listen for a response.
*/

OxideQQuickScriptMessageRequest* OxideQQuickWebFrame::sendMessage(
    const QUrl& context,
    const QString& msgId,
    const QVariant& payload) {
  Q_D(OxideQQuickWebFrame);

  OxideQQuickScriptMessageRequest* request =
      new OxideQQuickScriptMessageRequest();

  QVariant aux = payload;
  if (aux.userType() == qMetaTypeId<QJSValue>()) {
    aux = aux.value<QJSValue>().toVariant();
  }

  if (!d->proxy_->sendMessage(context, msgId, aux, request)) {
    delete request;
    return nullptr;
  }

  return request;
}

/*!
\qmlmethod void WebFrame::sendMessageNoReply(url context,
                                             string msgId,
                                             variant payload)

Send a message to the handler for \a{msgId} provided by the user script injected
in to the JS context identified by \a{context}. The message payload is supplied
via \{payload}. \a{payload} must be a value, object or array that can be
represented by JSON values.
*/

void OxideQQuickWebFrame::sendMessageNoReply(const QUrl& context,
                                             const QString& msgId,
                                             const QVariant& payload) {
  Q_D(OxideQQuickWebFrame);

  QVariant aux = payload;
  if (aux.userType() == qMetaTypeId<QJSValue>()) {
    aux = aux.value<QJSValue>().toVariant();
  }

  d->proxy_->sendMessageNoReply(context, msgId, aux);
}
