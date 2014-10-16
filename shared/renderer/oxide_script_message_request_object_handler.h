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

#ifndef _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_REQUEST_OBJECT_HANDLER_H_
#define _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_REQUEST_OBJECT_HANDLER_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "v8/include/v8.h"

#include "shared/renderer/oxide_object_backed_native_handler.h"

namespace oxide {

class ScriptMessageManager;

class ScriptMessageRequestObjectHandler final :
    public ObjectBackedNativeHandler {
 public:
  ScriptMessageRequestObjectHandler(ScriptMessageManager* mm);

 private:
  void GetOnReply(const v8::PropertyCallbackInfo<v8::Value>& info);
  void SetOnReply(v8::Local<v8::Value> value,
                  const v8::PropertyCallbackInfo<void>& info);

  void GetOnError(const v8::PropertyCallbackInfo<v8::Value>& info);
  void SetOnError(v8::Local<v8::Value> value,
                  const v8::PropertyCallbackInfo<void>& info);

  DISALLOW_IMPLICIT_CONSTRUCTORS(ScriptMessageRequestObjectHandler);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_REQUEST_OBJECT_HANDLER_H_
