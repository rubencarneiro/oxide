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

#ifndef _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_H_
#define _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_H_

#include <string>

#include "base/macros.h"
#include "v8/include/v8.h"

#include "shared/common/oxide_script_message.h"

#include "shared/renderer/oxide_script_referenced_object.h"

namespace oxide {

class ScriptMessageManager;

class ScriptMessageImplRenderer
    : public ScriptMessage,
      public ScriptReferencedObject<ScriptMessageImplRenderer> {
 public:
  ScriptMessageImplRenderer(ScriptMessageManager* mm,
                            int serial,
                            bool want_reply,
                            const std::string& msg_id,
                            base::ListValue* wrapped_payload,
                            const v8::Handle<v8::Object>& handle);

 private:
  ~ScriptMessageImplRenderer();

  // ScriptMessage implementation
  void DoSendResponse(const ScriptMessageParams& params) override;

  DISALLOW_COPY_AND_ASSIGN(ScriptMessageImplRenderer);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_H_
