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

#ifndef _OXIDE_SHARED_COMMON_SCRIPT_MESSAGE_HANDLER_H_
#define _OXIDE_SHARED_COMMON_SCRIPT_MESSAGE_HANDLER_H_

#include <memory>
#include <string>
#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "url/gurl.h"

#include "shared/common/oxide_shared_export.h"

namespace base {
class Value;
}

namespace oxide {

class ScriptMessage;

class OXIDE_SHARED_EXPORT ScriptMessageHandler final {
 public:
  typedef base::Callback<bool(ScriptMessage*, std::unique_ptr<base::Value>*)>
      HandlerCallback;

  ScriptMessageHandler();

  std::string msg_id() const {
    return msg_id_;
  }
  void set_msg_id(const std::string& id) {
    msg_id_ = id;
  }

  const std::vector<GURL>& contexts() const {
    return contexts_;
  }
  void set_contexts(const std::vector<GURL>& contexts) {
    contexts_ = contexts;
  }

  bool IsValid() const;

  void SetCallback(const HandlerCallback& callback);

  void OnReceiveMessage(ScriptMessage* message) const;

 private:
  std::string msg_id_;
  std::vector<GURL> contexts_;
  HandlerCallback callback_;

  DISALLOW_COPY_AND_ASSIGN(ScriptMessageHandler);
};

} // namespace oxide

#endif // _OXIDE_SHARED_COMMON_SCRIPT_MESSAGE_HANDLER_H_
