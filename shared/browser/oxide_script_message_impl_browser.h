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

#ifndef _OXIDE_SHARED_BROWSER_SCRIPT_MESSAGE_H_
#define _OXIDE_SHARED_BROWSER_SCRIPT_MESSAGE_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/weak_ptr.h"

#include "shared/common/oxide_script_message.h"

class GURL;
class OxideMsg_SendMessage_Params;

namespace oxide {

class WebFrame;

class ScriptMessageImplBrowser FINAL : public ScriptMessage {
 public:
  ScriptMessageImplBrowser(WebFrame* source_frame,
                           int serial,
                           const GURL& context,
                           const std::string& msg_is,
                           const std::string& args);
    
  WebFrame* source_frame() const { return source_frame_.get(); }

 private:
  void MakeParams(OxideMsg_SendMessage_Params* params);
  void SendResponse(const OxideMsg_SendMessage_Params& params);

  void DoSendReply(const std::string& args) FINAL;
  void DoSendError(ScriptMessageRequest::Error code,
                   const std::string& msg) FINAL;

  base::WeakPtr<WebFrame> source_frame_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(ScriptMessageImplBrowser);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_SCRIPT_MESSAGE_H_
