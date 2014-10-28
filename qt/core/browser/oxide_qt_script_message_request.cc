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

#include "oxide_qt_script_message_request.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QString>
#include <QVariant>

#include "base/bind.h"

#include "qt/core/glue/oxide_qt_script_message_request_adapter.h"
#include "shared/browser/oxide_script_message_request_impl_browser.h"

namespace oxide {
namespace qt {

ScriptMessageRequest::ScriptMessageRequest(
    ScriptMessageRequestAdapter* adapter)
    : adapter_(adapter) {}

void ScriptMessageRequest::ReceiveReplyCallback(const std::string& args) {
  QJsonDocument jsondoc(QJsonDocument::fromJson(
      QByteArray(args.data(), args.length())));

  adapter_->OnReceiveReply(jsondoc.toVariant());
}

void ScriptMessageRequest::ReceiveErrorCallback(
    oxide::ScriptMessageRequest::Error error,
    const std::string& msg) {
  adapter_->OnReceiveError(error, QString::fromStdString(msg));
}

ScriptMessageRequest::~ScriptMessageRequest() {}

// static
ScriptMessageRequest* ScriptMessageRequest::FromAdapter(
    ScriptMessageRequestAdapter* adapter) {
  return adapter->request_.data();
}

void ScriptMessageRequest::SetRequest(
    oxide::ScriptMessageRequestImplBrowser* req) {
  DCHECK(!request_ && req);
  request_.reset(req);

  request_->SetReplyCallback(
      base::Bind(
        &ScriptMessageRequest::ReceiveReplyCallback,
        base::Unretained(this)));
  request_->SetErrorCallback(
      base::Bind(
        &ScriptMessageRequest::ReceiveErrorCallback,
        base::Unretained(this)));
}

} // namespace qt
} // namespace oxide
