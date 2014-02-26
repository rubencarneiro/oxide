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

#include "oxideqscriptmessage.h"
#include "oxideqscriptmessage_p.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QString>

#include "base/logging.h"

#include "shared/browser/oxide_script_message_impl_browser.h"
#include "shared/common/oxide_script_message_request.h"

OxideQScriptMessagePrivate::OxideQScriptMessagePrivate() :
    incoming_(NULL),
    weak_factory_(this) {}

void OxideQScriptMessagePrivate::Initialize(oxide::ScriptMessage* message) {
  DCHECK(!incoming());
  incoming_ = static_cast<oxide::ScriptMessageImplBrowser *>(message);

  QJsonDocument jsondoc(QJsonDocument::fromJson(
      QByteArray(message->args().data(), message->args().length())));
  args_ = jsondoc.toVariant();
}

void OxideQScriptMessagePrivate::Consume(
    scoped_ptr<oxide::ScriptMessage> message) {
  DCHECK_EQ(incoming(), message.get());
  DCHECK(!owned_incoming_);
  owned_incoming_ = message.Pass();
}

void OxideQScriptMessagePrivate::Invalidate() {
  incoming_ = NULL;
}

// static
OxideQScriptMessagePrivate* OxideQScriptMessagePrivate::get(
    OxideQScriptMessage* q) {
  return q->d_func();
}

OxideQScriptMessage::OxideQScriptMessage() :
    d_ptr(new OxideQScriptMessagePrivate()) {}

OxideQScriptMessage::~OxideQScriptMessage() {}

QUrl OxideQScriptMessage::context() const {
  Q_D(const OxideQScriptMessage);

  oxide::ScriptMessageImplBrowser* message = d->incoming();
  if (!message) {
    return QUrl();
  }

  return QUrl(QString::fromStdString(message->context().spec()));
}

QVariant OxideQScriptMessage::args() const {
  Q_D(const OxideQScriptMessage);

  return d->args();
}

void OxideQScriptMessage::reply(const QVariant& args) {
  Q_D(OxideQScriptMessage);

  oxide::ScriptMessageImplBrowser* message = d->incoming();
  if (!message) {
    return;
  }

  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));
  message->Reply(QString(jsondoc.toJson()).toStdString());
}

void OxideQScriptMessage::error(const QString& msg) {
  Q_D(OxideQScriptMessage);

  oxide::ScriptMessageImplBrowser* message = d->incoming();
  if (!message) {
    return;
  }

  message->Error(
      oxide::ScriptMessageRequest::ERROR_HANDLER_REPORTED_ERROR,
      msg.toStdString());
}
