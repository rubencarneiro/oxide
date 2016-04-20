// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include <utility>

#include <QByteArray>
#include <QJsonDocument>
#include <QString>
#include <QVariant>

#include "base/bind.h"

#include "qt/core/glue/oxide_qt_script_message_request_proxy_client.h"
#include "shared/browser/oxide_script_message_request_impl_browser.h"

#include "oxide_qt_variant_value_converter.h"

namespace oxide {
namespace qt {

void ScriptMessageRequest::ReceiveReplyCallback(const base::Value& payload) {
  client_->ReceiveReply(VariantValueConverter::ToVariantValue(&payload));
}

void ScriptMessageRequest::ReceiveErrorCallback(
    oxide::ScriptMessageParams::Error error,
    const base::Value& payload) {
  client_->ReceiveError(
      error,
      VariantValueConverter::ToVariantValue(&payload));
}

ScriptMessageRequest::ScriptMessageRequest(
    ScriptMessageRequestProxyClient* client,
    QObject* handle)
    : client_(client) {
  DCHECK(client);
  DCHECK(handle);

  setHandle(handle);
}

ScriptMessageRequest::~ScriptMessageRequest() {}

void ScriptMessageRequest::SetRequest(
    std::unique_ptr<oxide::ScriptMessageRequestImplBrowser> req) {
  DCHECK(!request_ && req);
  request_ = std::move(req);

  request_->SetReplyCallback(
      base::Bind(
        &ScriptMessageRequest::ReceiveReplyCallback,
        // The callback cannot run after |this| is deleted, as it exclusively
        // owns |request_|
        base::Unretained(this)));
  request_->SetErrorCallback(
      base::Bind(
        &ScriptMessageRequest::ReceiveErrorCallback,
        // The callback cannot run after |this| is deleted, as it exclusively
        // owns |request_|
        base::Unretained(this)));
}

} // namespace qt
} // namespace oxide
