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

#ifndef _OXIDE_QT_CORE_BROWSER_SCRIPT_MESSAGE_REQUEST_H_
#define _OXIDE_QT_CORE_BROWSER_SCRIPT_MESSAGE_REQUEST_H_

#include <memory>
#include <string>

#include "base/macros.h"

#include "qt/core/glue/oxide_qt_script_message_request_proxy.h"
#include "shared/common/oxide_script_message_params.h"

namespace base {
class Value;
}

namespace oxide {

class ScriptMessageRequestImplBrowser;

namespace qt {

class ScriptMessageRequestProxyClient;

class ScriptMessageRequest : public ScriptMessageRequestProxy {
 public:
  ScriptMessageRequest(ScriptMessageRequestProxyClient* client,
                       QObject* handle);
  ~ScriptMessageRequest() override;

  void SetRequest(std::unique_ptr<oxide::ScriptMessageRequestImplBrowser> req);

 private:
  void ReceiveReplyCallback(const base::Value& payload);
  void ReceiveErrorCallback(oxide::ScriptMessageParams::Error error,
                            const base::Value& payload);

  ScriptMessageRequestProxyClient* client_;
  std::unique_ptr<oxide::ScriptMessageRequestImplBrowser> request_;

  DISALLOW_COPY_AND_ASSIGN(ScriptMessageRequest);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_SCRIPT_MESSAGE_REQUEST_H_
