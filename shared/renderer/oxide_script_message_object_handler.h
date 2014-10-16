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

#ifndef _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_OBJECT_HANDLER_H_
#define _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_OBJECT_HANDLER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "v8/include/v8.h"

#include "shared/renderer/oxide_object_backed_native_handler.h"
#include "shared/renderer/oxide_v8_scoped_persistent.h"

namespace oxide {

class ScriptMessageManager;

class ScriptMessageObjectHandler final : public ObjectBackedNativeHandler {
 public:
  ScriptMessageObjectHandler(ScriptMessageManager* mm);

 private:
  void Reply(const v8::FunctionCallbackInfo<v8::Value>& info);
  void Error(const v8::FunctionCallbackInfo<v8::Value>& info);

  void GetID(const v8::PropertyCallbackInfo<v8::Value>& info);
  void GetArgs(const v8::PropertyCallbackInfo<v8::Value>& info);

  v8::Handle<v8::String> Stringify(v8::Isolate* isolate,
                                   v8::Handle<v8::Object> object);

  ScopedPersistent<v8::Function> stringify_func_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(ScriptMessageObjectHandler);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_OBJECT_HANDLER_H_
