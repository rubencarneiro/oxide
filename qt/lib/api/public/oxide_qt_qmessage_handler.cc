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

#include "oxide_q_message_handler_base.h"
#include "oxide_qquick_message_handler_p.h"

#include <QtDebug>

#include "qt/lib/api/oxide_qt_qmessage_handler_p.h"

OxideQMessageHandlerBase::OxideQMessageHandlerBase(
    oxide::qt::QMessageHandlerBasePrivate& dd,
    QObject* parent) :
    QObject(parent),
    d_ptr(&dd) {}

OxideQMessageHandlerBase::~OxideQMessageHandlerBase() {}

QString OxideQMessageHandlerBase::msgId() const {
  Q_D(const oxide::qt::QMessageHandlerBase);

  return QString::fromStdString(d->handler()->msg_id());
}

void OxideQMessageHandlerBase::setMsgId(const QString& id) {
  Q_D(oxide::qt::QMessageHandlerBase);

  if (id.toStdString() == d->handler()->msg_id()) {
    return;
  }

  d->handler()->set_msg_id(id.toStdString());
  emit msgIdChanged();
}

OxideQQuickMessageHandler::OxideQQuickMessageHandler(QObject* parent) :
    OxideQMessageHandlerBase(
      *oxide::qt::QQuickMessageHandlerPrivate::Create(this),
       parent) {}

OxideQQuickMessageHandler::~OxideQQuickMessageHandler() {}

QJSValue OxideQQuickMessageHandler::callback() const {
  Q_D(const oxide::qt::QQuickMessageHandler);

  return d->callback;
}

void OxideQQuickMessageHandler::setCallback(const QJSValue& callback) {
  Q_D(oxide::qt::QQuickMessageHandler);

  if (callback.strictlyEquals(d->callback)) {
    return;
  }

  if (!callback.isCallable()) {
    qWarning() << "Invalid callback";
    return;
  }

  d->callback = callback;
  emit callbackChanged();
}
