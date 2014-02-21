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

#include "oxideqincomingmessage.h"
#include "oxideqincomingmessage_p.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QString>

#include "shared/browser/oxide_incoming_message.h"

OxideQIncomingMessagePrivate::OxideQIncomingMessagePrivate() {}

void OxideQIncomingMessagePrivate::Initialize(
    oxide::IncomingMessage* message) {
  Q_ASSERT(!incoming());
  incoming_.reset(message);

  QJsonDocument jsondoc(QJsonDocument::fromJson(
      QByteArray(message->args().data(), message->args().length())));
  args_ = jsondoc.toVariant();
}

// static
OxideQIncomingMessagePrivate* OxideQIncomingMessagePrivate::get(
    OxideQIncomingMessage* q) {
  return q->d_func();
}

OxideQIncomingMessage::OxideQIncomingMessage() :
    d_ptr(new OxideQIncomingMessagePrivate()) {}

OxideQIncomingMessage::~OxideQIncomingMessage() {}

QUrl OxideQIncomingMessage::context() const {
  Q_D(const OxideQIncomingMessage);

  return QUrl(QString::fromStdString(d->incoming()->context().spec()));
}

QVariant OxideQIncomingMessage::args() const {
  Q_D(const OxideQIncomingMessage);

  return d->args();
}

void OxideQIncomingMessage::reply(const QVariant& args) {
  Q_D(OxideQIncomingMessage);

  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));
  d->incoming()->Reply(QString(jsondoc.toJson()).toStdString());
}

void OxideQIncomingMessage::error(const QString& msg) {
  Q_D(OxideQIncomingMessage);

  d->incoming()->Error(msg.toStdString());
}
