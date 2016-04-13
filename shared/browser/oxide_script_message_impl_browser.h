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

#ifndef _OXIDE_SHARED_BROWSER_SCRIPT_MESSAGE_H_
#define _OXIDE_SHARED_BROWSER_SCRIPT_MESSAGE_H_

#include <string>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"

#include "shared/common/oxide_script_message.h"
#include "shared/common/oxide_shared_export.h"

class GURL;

namespace base {
class ListValue;
}

namespace oxide {

class WebFrame;

class OXIDE_SHARED_EXPORT ScriptMessageImplBrowser : public ScriptMessage {
 public:
  ScriptMessageImplBrowser(WebFrame* source_frame,
                           int serial,
                           const GURL& context,
                           const std::string& msg_is,
                           base::ListValue* wrapped_payload);

  WebFrame* source_frame() const { return source_frame_.get(); }  

 private:
  ~ScriptMessageImplBrowser();

  // ScriptMessage implementation
  void DoSendResponse(const ScriptMessageParams& params) override;

  base::WeakPtr<WebFrame> source_frame_;

  DISALLOW_COPY_AND_ASSIGN(ScriptMessageImplBrowser);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_SCRIPT_MESSAGE_H_
