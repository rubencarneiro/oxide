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

#include "qt/core/browser/oxide_qt_script_message_request.h"
#include "qt/core/browser/oxide_qt_web_frame.h"

namespace oxide {
namespace qt {

WebFrameAdapter::WebFrameAdapter(QObject* q) :
    AdapterBase(q),
    frame_(NULL) {}

WebFrameAdapter::~WebFrameAdapter() {}

// static
WebFrameAdapter* WebFrameAdapter::FromWebFrame(WebFrame* frame) {
  if (!frame) {
    return NULL;
  }

  return frame->adapter_;
}

QUrl WebFrameAdapter::url() const {
  return QUrl(QString::fromStdString(frame_->GetURL().spec()));
}

bool WebFrameAdapter::sendMessage(const QUrl& context,
                                  const QString& msg_id,
                                  const QVariant& args,
                                  ScriptMessageRequestAdapter* req) {
  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));

  return frame_->SendMessage(GURL(context.toString().toStdString()),
                             msg_id.toStdString(),
                             QString(jsondoc.toJson()).toStdString(),
                             ScriptMessageRequest::FromAdapter(req));
}

void WebFrameAdapter::sendMessageNoReply(const QUrl& context,
                                         const QString& msg_id,
                                         const QVariant& args) {
  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));

  frame_->SendMessageNoReply(
      GURL(context.toString().toStdString()),
      msg_id.toStdString(),
      QString(jsondoc.toJson()).toStdString());
}

QList<ScriptMessageHandlerAdapter *>& WebFrameAdapter::messageHandlers() {
  return message_handlers_;
}

} // namespace qt
} // namespace oxide
