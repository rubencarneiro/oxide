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

#include "oxide_qt_web_frame_adapter.h"

#include <QJsonDocument>
#include <QString>
#include <QVariant>

#include "url/gurl.h"

#include "qt/core/browser/oxide_qt_web_frame.h"
#include "qt/core/glue/private/oxide_qt_outgoing_message_request_adapter_p.h"
#include "qt/core/glue/private/oxide_qt_web_frame_adapter_p.h"

namespace oxide {
namespace qt {

WebFrameAdapter::WebFrameAdapter() :
    priv_(WebFrameAdapterPrivate::Create()) {}

WebFrameAdapter::~WebFrameAdapter() {}

QUrl WebFrameAdapter::url() const {
  return QUrl(QString::fromStdString(priv_->owner->url().spec()));
}

bool WebFrameAdapter::sendMessage(const QString& world_id,
                                  const QString& msg_id,
                                  const QVariant& args,
                                  OutgoingMessageRequestAdapter* req) {
  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));

  if (priv_->owner->SendMessage(
      world_id.toStdString(),
      msg_id.toStdString(),
      QString(jsondoc.toJson()).toStdString(),
      &OutgoingMessageRequestAdapterPrivate::get(req)->request())) {
    priv_->AddOutgoingMessageRequest(req);
    return true;
  }

  return false;
}

void WebFrameAdapter::sendMessageNoReply(const QString& world_id,
                                         const QString& msg_id,
                                         const QVariant& args) {
  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));

  priv_->owner->SendMessageNoReply(
      world_id.toStdString(),
      msg_id.toStdString(),
      QString(jsondoc.toJson()).toStdString());
}

QList<MessageHandlerAdapter *>& WebFrameAdapter::message_handlers() {
  return priv_->message_handlers();
}

} // namespace qt
} // namespace oxide
