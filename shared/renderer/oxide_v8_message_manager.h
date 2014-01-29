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

#ifndef _OXIDE_SHARED_RENDERER_V8_MESSAGE_MANAGER_H_
#define _OXIDE_SHARED_RENDERER_V8_MESSAGE_MANAGER_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "v8/include/v8.h"

#include "oxide_v8_scoped_persistent.h"

struct OxideMsg_SendMessage_Params;

namespace content {
class RenderFrame;
class RenderView;
}

namespace oxide {

class V8MessageManager FINAL {
 public:
  V8MessageManager(content::RenderFrame* frame,
                   v8::Handle<v8::Context> context,
                   int world_id);
  ~V8MessageManager();

  void ReceiveMessage(const OxideMsg_SendMessage_Params& params);

  v8::Handle<v8::Context> v8_context() const;
  int world_id() const { return world_id_; }

 private:
  static std::string V8StringToStdString(v8::Local<v8::String> string);

  static V8MessageManager* GetMessageManagerFromArgs(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  static void SendMessage(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void RegisterReceiveHandler(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  void SendMessageInner(const v8::FunctionCallbackInfo<v8::Value>& args);
  void RegisterReceiveHandlerInner(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  static void OxideLazyGetter(v8::Local<v8::String> property,
                              const v8::PropertyCallbackInfo<v8::Value>& info);

  void OxideLazyGetterInner(v8::Local<v8::String> property,
                            const v8::PropertyCallbackInfo<v8::Value>& info);

  content::RenderFrame* frame_;
  ScopedPersistent<v8::Context> context_;
  int world_id_;
  ScopedPersistent<v8::External> closure_data_;

  ScopedPersistent<v8::Function> receive_handler_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(V8MessageManager);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_V8_MESSAGE_MANAGER_H_
