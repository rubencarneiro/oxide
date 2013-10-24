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

#include "oxideqquickmessagehandler_p.h"
#include "oxideqquickmessagehandler_p_p.h"

#include <QtDebug>

#include "oxideqquickwebframe_p.h"
#include "oxideqquickwebview_p.h"

OxideQQuickMessageHandler::OxideQQuickMessageHandler(QObject* parent) :
    QObject(parent),
    d_ptr(OxideQQuickMessageHandlerPrivate::Create(this)) {}

OxideQQuickMessageHandler::~OxideQQuickMessageHandler() {
  Q_D(OxideQQuickMessageHandler);

  d->removeFromCurrentOwner();
}

QString OxideQQuickMessageHandler::msgId() const {
  Q_D(const OxideQQuickMessageHandler);

  return d->msgId();
}

void OxideQQuickMessageHandler::setMsgId(const QString& id) {
  Q_D(OxideQQuickMessageHandler);

  if (id == d->msgId()) {
    return;
  }

  d->setMsgId(id);
  emit msgIdChanged();
}

QList<QString> OxideQQuickMessageHandler::worldIds() const {
  Q_D(const OxideQQuickMessageHandler);

  return d->worldIds();
}

void OxideQQuickMessageHandler::setWorldIds(const QList<QString>& ids) {
  Q_D(OxideQQuickMessageHandler);

  d->setWorldIds(ids);
  emit worldIdsChanged();
}

QJSValue OxideQQuickMessageHandler::callback() const {
  Q_D(const OxideQQuickMessageHandler);

  return d->callback;
}

void OxideQQuickMessageHandler::setCallback(const QJSValue& callback) {
  Q_D(OxideQQuickMessageHandler);

  if (callback.strictlyEquals(d->callback)) {
    return;
  }

  bool is_null = callback.isNull() || callback.isUndefined();

  if (!callback.isCallable() && !is_null) {
    qWarning() << "Invalid callback";
    return;
  }

  d->callback = callback;

  if (is_null) {
    d->detachHandler();
  } else {
    d->attachHandler();
  }

  emit callbackChanged();
}

void OxideQQuickMessageHandler::classBegin() {}

void OxideQQuickMessageHandler::componentComplete() {
  if (OxideQQuickWebView* view = qobject_cast<OxideQQuickWebView *>(parent())) {
    view->addMessageHandler(this);
  } else if (OxideQQuickWebFrame* frame =
             qobject_cast<OxideQQuickWebFrame *>(parent())) {
    frame->addMessageHandler(this);
  }
}
