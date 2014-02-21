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
#include "oxide_qt_web_frame_adapter_p.h"

#include <QJsonDocument>
#include <QString>
#include <QVariant>

#include "url/gurl.h"

#include "qt/core/browser/oxide_qt_web_frame.h"
#include "qt/core/glue/private/oxide_qt_outgoing_message_request_adapter_p.h"

namespace oxide {
namespace qt {

WebFrameAdapterPrivate::WebFrameAdapterPrivate() :
    owner(NULL) {}

WebFrameAdapterPrivate::~WebFrameAdapterPrivate() {
  while (!outgoing_message_requests_.isEmpty()) {
    RemoveOutgoingMessageRequest(outgoing_message_requests_.first());
  }
}

void WebFrameAdapterPrivate::AddOutgoingMessageRequest(
    OutgoingMessageRequestAdapter* request) {
  DCHECK(!outgoing_message_requests_.contains(request));

  OutgoingMessageRequestAdapterPrivate::get(request)->frame = this;
  outgoing_message_requests_.append(request);
}

void WebFrameAdapterPrivate::RemoveOutgoingMessageRequest(
    OutgoingMessageRequestAdapter* request) {
  outgoing_message_requests_.removeOne(request);
  OutgoingMessageRequestAdapterPrivate::get(request)->frame = NULL;
}

// static
WebFrameAdapterPrivate* WebFrameAdapterPrivate::get(WebFrameAdapter* adapter) {
  return adapter->priv.data();
}

WebFrameAdapter::WebFrameAdapter(QObject* q) :
    AdapterBase(q),
    priv(new WebFrameAdapterPrivate()) {}

WebFrameAdapter::~WebFrameAdapter() {}

QUrl WebFrameAdapter::url() const {
  return QUrl(QString::fromStdString(priv->owner->url().spec()));
}

bool WebFrameAdapter::sendMessage(const QUrl& context,
                                  const QString& msg_id,
                                  const QVariant& args,
                                  OutgoingMessageRequestAdapter* req) {
  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));

  if (priv->owner->SendMessage(
      GURL(context.toString().toStdString()),
      msg_id.toStdString(),
      QString(jsondoc.toJson()).toStdString(),
      &OutgoingMessageRequestAdapterPrivate::get(req)->request())) {
    priv->AddOutgoingMessageRequest(req);
    return true;
  }

  return false;
}

void WebFrameAdapter::sendMessageNoReply(const QUrl& context,
                                         const QString& msg_id,
                                         const QVariant& args) {
  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));

  priv->owner->SendMessageNoReply(
      GURL(context.toString().toStdString()),
      msg_id.toStdString(),
      QString(jsondoc.toJson()).toStdString());
}

} // namespace qt
} // namespace oxide
