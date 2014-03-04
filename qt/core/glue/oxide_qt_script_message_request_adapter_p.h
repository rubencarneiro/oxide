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

#ifndef _OXIDE_QT_CORE_GLUE_PRIVATE_SCRIPT_MESSAGE_REQUEST_ADAPTER_H_
#define _OXIDE_QT_CORE_GLUE_PRIVATE_SCRIPT_MESSAGE_REQUEST_ADAPTER_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"

#include "shared/browser/oxide_script_message_request_impl_browser.h"

namespace oxide {

class ScriptMessageRequestImplBrowser;

namespace qt {

class ScriptMessageRequestAdapter;

class ScriptMessageRequestAdapterPrivate FINAL {
 public:
  ScriptMessageRequestAdapterPrivate(ScriptMessageRequestAdapter* adapter);

  static ScriptMessageRequestAdapterPrivate* get(
      ScriptMessageRequestAdapter* adapter);

  void SetRequest(oxide::ScriptMessageRequestImplBrowser* req);

 private:
  friend class ScriptMessageRequestAdapter;

  void ReceiveReplyCallback(const std::string& args);
  void ReceiveErrorCallback(oxide::ScriptMessageRequest::Error error,
                            const std::string& msg);

  ScriptMessageRequestAdapter* a;
  scoped_ptr<oxide::ScriptMessageRequestImplBrowser> request_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(ScriptMessageRequestAdapterPrivate);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_PRIVATE_SCRIPT_MESSAGE_REQUEST_ADAPTER_H_
