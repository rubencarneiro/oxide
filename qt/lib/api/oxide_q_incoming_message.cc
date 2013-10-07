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

#include "oxide_q_incoming_message.h"

#include <QByteArray>
#include <QJsonDocument>

#include "shared/browser/oxide_incoming_message.h"

#include "qt/lib/api/private/oxide_q_incoming_message_p.h"

OxideQIncomingMessage::OxideQIncomingMessage(
    oxide::IncomingMessage* message) :
    QObject(),
    d_ptr(oxide::qt::QIncomingMessagePrivate::Create(message)) {}

OxideQIncomingMessage::~OxideQIncomingMessage() {}

QString OxideQIncomingMessage::worldId() const {
  Q_D(const oxide::qt::QIncomingMessage);

  return QString::fromStdString(d->incoming()->world_id());
}

QVariant OxideQIncomingMessage::args() const {
  Q_D(const oxide::qt::QIncomingMessage);

  return d->args();
}

void OxideQIncomingMessage::reply(const QVariant& args) {
  Q_D(oxide::qt::QIncomingMessage);

  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));
  d->incoming()->Reply(QString(jsondoc.toJson()).toStdString());
}

void OxideQIncomingMessage::error(const QString& msg) {
  Q_D(oxide::qt::QIncomingMessage);

  d->incoming()->Error(msg.toStdString());
}
