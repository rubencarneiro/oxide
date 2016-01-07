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

#ifndef _OXIDE_SHARED_RENDERER_OBJECT_BACKED_NATIVE_HANDLER_H_
#define _OXIDE_SHARED_RENDERER_OBJECT_BACKED_NATIVE_HANDLER_H_

#include <string>

#include "base/callback.h"
#include "base/macros.h"
#include "v8/include/v8.h"
#include "v8/include/v8-util.h"

#include "shared/renderer/oxide_v8_scoped_persistent.h"

namespace oxide {

class ScriptMessageManager;

class ObjectBackedNativeHandler {
 public:
  ObjectBackedNativeHandler(ScriptMessageManager* manager);
  virtual ~ObjectBackedNativeHandler();

  v8::Handle<v8::Object> NewInstance();

 protected:
  typedef base::Callback<void(
      const v8::FunctionCallbackInfo<v8::Value>&)> HandlerFunction;
  typedef base::Callback<void(
      const v8::PropertyCallbackInfo<v8::Value>&)> HandlerGetter;
  typedef base::Callback<void(
      v8::Local<v8::Value>,
      const v8::PropertyCallbackInfo<void>&)> HandlerSetter; 
 
  void RouteFunction(const std::string& name,
                     const HandlerFunction& handler_function);
  void RouteAccessor(const std::string& name,
                     const HandlerGetter& getter,
                     const HandlerSetter& setter = HandlerSetter(),
                     v8::PropertyAttribute attr = v8::None);

 private:
  static void FunctionRouter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetterRouter(v8::Local<v8::String> prop,
                           const v8::PropertyCallbackInfo<v8::Value>& info);
  static void SetterRouter(v8::Local<v8::String> prop,
                           v8::Local<v8::Value> value,
                           const v8::PropertyCallbackInfo<void>& info);

  ScriptMessageManager* manager_;

  typedef v8::PersistentValueVector<v8::Object> RouterData;
  RouterData router_data_;

  ScopedPersistent<v8::ObjectTemplate> object_template_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(ObjectBackedNativeHandler);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_OBJECT_BACKED_NATIVE_HANDLER_H_
