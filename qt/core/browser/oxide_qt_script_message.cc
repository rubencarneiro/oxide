// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#include "oxide_qt_script_message.h"

#include "base/logging.h"

#include "shared/browser/oxide_script_message_impl_browser.h"
#include "shared/common/oxide_script_message_params.h"

#include "oxide_qt_variant_value_converter.h"
#include "oxide_qt_web_frame.h"

namespace oxide {
namespace qt {

WebFrameProxyHandle* ScriptMessage::frame() const {
  WebFrame* frame = WebFrame::FromSharedWebFrame(impl_->source_frame());
  if (!frame) {
    return nullptr;
  }

  return frame->handle();
}

QString ScriptMessage::msgId() const {
  return QString::fromStdString(impl_->msg_id());
}

QUrl ScriptMessage::context() const {
  return QUrl(QString::fromStdString(impl_->context().spec()));
}

QVariant ScriptMessage::payload() const {
  return payload_;
}

void ScriptMessage::reply(const QVariant& payload) {
  impl_->Reply(VariantValueConverter::FromVariantValue(payload));
}

void ScriptMessage::error(const QVariant& payload) {
  impl_->Error(oxide::ScriptMessageParams::ERROR_HANDLER_REPORTED_ERROR,
               VariantValueConverter::FromVariantValue(payload));
}

ScriptMessage::ScriptMessage(oxide::ScriptMessage* message)
    : impl_(static_cast<oxide::ScriptMessageImplBrowser*>(message)),
      payload_(VariantValueConverter::ToVariantValue(message->payload())) {}

ScriptMessage::~ScriptMessage() {}

// static
ScriptMessage* ScriptMessage::FromProxyHandle(ScriptMessageProxyHandle* handle) {
  return static_cast<ScriptMessage*>(handle->proxy_.data());
}

} // namespace qt
} // namespace oxide
