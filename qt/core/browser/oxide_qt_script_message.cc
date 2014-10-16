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

#include "oxide_qt_script_message.h"

#include "base/logging.h"

#include "qt/core/glue/oxide_qt_script_message_adapter.h"
#include "shared/browser/oxide_script_message_impl_browser.h"
#include "shared/common/oxide_script_message_request.h"

#include "oxide_qt_web_frame.h"

namespace oxide {
namespace qt {

ScriptMessage::ScriptMessage(ScriptMessageAdapter* adapter)
    : adapter_(adapter) {}

ScriptMessage::~ScriptMessage() {}

void ScriptMessage::Initialize(oxide::ScriptMessage* message) {
  DCHECK(!impl_.get());
  impl_ = static_cast<oxide::ScriptMessageImplBrowser*>(message);
}

// static
ScriptMessage* ScriptMessage::FromAdapter(ScriptMessageAdapter* adapter) {
  return adapter->message_.data();
}

WebFrame* ScriptMessage::GetFrame() const {
  return static_cast<WebFrame*>(impl_->source_frame());
}

std::string ScriptMessage::GetMsgId() const {
  return impl_->msg_id();
}

GURL ScriptMessage::GetContext() const {
  return impl_->context();
}

std::string ScriptMessage::GetArgs() const {
  return impl_->args();
}

void ScriptMessage::Reply(const std::string& args) {
  impl_->Reply(args);
}

void ScriptMessage::Error(const std::string& msg) {
  impl_->Error(
      oxide::ScriptMessageRequest::ERROR_HANDLER_REPORTED_ERROR,
      msg);
}

} // namespace qt
} // namespace oxide
