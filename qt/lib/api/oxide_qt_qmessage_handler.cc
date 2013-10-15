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

#include "qt/lib/api/private/oxide_qt_qmessage_handler_p.h"

#include "oxide_qquick_web_frame_p.h"
#include "oxide_qquick_web_view_p.h"

OxideQMessageHandlerBase::OxideQMessageHandlerBase(
    oxide::qt::QMessageHandlerBasePrivate& dd,
    QObject* parent) :
    QObject(parent),
    d_ptr(&dd) {}

OxideQMessageHandlerBase::~OxideQMessageHandlerBase() {
  Q_D(oxide::qt::QMessageHandlerBase);

  d->removeFromCurrentOwner();
}

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

QList<QString> OxideQMessageHandlerBase::worldIds() const {
  Q_D(const oxide::qt::QMessageHandlerBase);

  QList<QString> list;

  const std::vector<std::string>& ids = d->handler()->world_ids();
  for (std::vector<std::string>::const_iterator it = ids.begin();
       it != ids.end(); ++it) {
    list.append(QString::fromStdString(*it));
  }

  return list;
}

void OxideQMessageHandlerBase::setWorldIds(const QList<QString>& ids) {
  Q_D(oxide::qt::QMessageHandlerBase);

  std::vector<std::string> list;

  for (int i = 0; i < ids.size(); ++i) {
    list.push_back(ids[i].toStdString());
  }

  d->handler()->set_world_ids(list);
  emit worldIdsChanged();
}

OxideQQuickMessageHandler::OxideQQuickMessageHandler(QObject* parent) :
    OxideQMessageHandlerBase(
      *oxide::qt::QQuickMessageHandlerPrivate::Create(this),
       parent) {}

OxideQQuickMessageHandler::~OxideQQuickMessageHandler() {}

void OxideQQuickMessageHandler::classBegin() {}

void OxideQQuickMessageHandler::componentComplete() {
  if (OxideQQuickWebView* view = qobject_cast<OxideQQuickWebView *>(parent())) {
    view->addMessageHandler(this);
  } else if (OxideQQuickWebFrame* frame =
             qobject_cast<OxideQQuickWebFrame *>(parent())) {
    frame->addMessageHandler(this);
  }
}

QJSValue OxideQQuickMessageHandler::callback() const {
  Q_D(const oxide::qt::QQuickMessageHandler);

  return d->callback;
}

void OxideQQuickMessageHandler::setCallback(const QJSValue& callback) {
  Q_D(oxide::qt::QQuickMessageHandler);

  if (callback.strictlyEquals(d->callback)) {
    return;
  }

  if (!callback.isCallable() &&
      !callback.isNull() &&
      !callback.isUndefined()) {
    qWarning() << "Invalid callback";
    return;
  }

  d->callback = callback;
  emit callbackChanged();
}
