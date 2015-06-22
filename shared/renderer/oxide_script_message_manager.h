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

#ifndef _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_MANAGER_H_
#define _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_MANAGER_H_

#include <map>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/weak_ptr.h"
#include "url/gurl.h"
#include "v8/include/v8.h"

#include "shared/renderer/oxide_script_message_object_handler.h"
#include "shared/renderer/oxide_script_message_request_object_handler.h"
#include "shared/renderer/oxide_v8_scoped_persistent.h"

namespace content {
class RenderFrame;
}

namespace oxide {

class ScriptMessageHandlerRenderer;
class ScriptMessageRequestImplRenderer;

class ScriptMessageManager
    : public base::SupportsWeakPtr<ScriptMessageManager> {
 public:
  typedef std::vector<ScriptMessageRequestImplRenderer*>
      ScriptMessageRequestVector;

  ScriptMessageManager(content::RenderFrame* frame,
                       v8::Handle<v8::Context> context,
                       int world_id);
  ~ScriptMessageManager();

  v8::Handle<v8::Context> GetV8Context() const;

  GURL GetContextURL() const;

  int world_id() const { return world_id_; }

  content::RenderFrame* frame() const { return frame_; }

  v8::Isolate* isolate() const { return isolate_; }

  const ScriptMessageRequestVector& current_script_message_requests() const {
    return current_script_message_requests_;
  }

  ScriptMessageObjectHandler& script_message_object_handler() {
    return script_message_object_handler_;
  }

  ScriptMessageHandlerRenderer* GetHandlerForMsgID(const std::string& msg_id);

  v8::Handle<v8::Object> GetOxideApiObject(v8::Isolate* isolate);

 private:
  friend class ScriptMessageRequestImplRenderer;
  typedef std::map<std::string, linked_ptr<ScriptMessageHandlerRenderer>>
      ScriptMessageHandlerMap;

  static std::string V8StringToStdString(v8::Local<v8::String> string);

  static ScriptMessageManager* GetMessageManagerFromArgs(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  static void OxideLazyGetter(v8::Local<v8::String> property,
                              const v8::PropertyCallbackInfo<v8::Value>& info);
  void OxideLazyGetterInner(v8::Local<v8::String> property,
                            const v8::PropertyCallbackInfo<v8::Value>& info);

  static void SendMessage(const v8::FunctionCallbackInfo<v8::Value>& args);
  void SendMessageInner(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void AddMessageHandler(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void AddMessageHandlerInner(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void RemoveMessageHandler(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void RemoveMessageHandlerInner(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  void AddScriptMessageRequest(ScriptMessageRequestImplRenderer* req);
  void RemoveScriptMessageRequest(ScriptMessageRequestImplRenderer* req);

  content::RenderFrame* frame_;
  v8::Isolate* isolate_;
  ScopedPersistent<v8::Context> context_;
  int world_id_;

  int next_message_id_;

  ScriptMessageRequestObjectHandler script_message_request_object_handler_;
  ScriptMessageRequestVector current_script_message_requests_;

  ScriptMessageHandlerMap script_message_handler_map_;

  ScriptMessageObjectHandler script_message_object_handler_;

  ScopedPersistent<v8::External> closure_data_;

  DISALLOW_COPY_AND_ASSIGN(ScriptMessageManager);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_MANAGER_H_
