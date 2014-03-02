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

#include "oxide_script_message_request_object_handler.h"

#include "base/bind.h"

#include "oxide_script_message_request_impl_renderer.h"
#include "oxide_script_referenced_object.h"

namespace oxide {

void ScriptMessageRequestObjectHandler::GetOnReply(
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  ScriptMessageRequestImplRenderer* req =
      ScriptReferencedObject<ScriptMessageRequestImplRenderer>::FromScriptHandle(
        info.Holder());
  if (!req) {
    return;
  }

  v8::Handle<v8::Function> callback(req->GetOnReplyCallback(isolate));
  if (callback.IsEmpty()) {
    info.GetReturnValue().Set(v8::Undefined(isolate));
    return;
  }

  info.GetReturnValue().Set(callback);
}

void ScriptMessageRequestObjectHandler::SetOnReply(
    v8::Local<v8::Value> value,
    const v8::PropertyCallbackInfo<void>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  if (!value->IsUndefined() && !value->IsFunction()) {
    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(
          isolate, "Unexpected type")));
    return;
  }

  ScriptMessageRequestImplRenderer* req =
      ScriptReferencedObject<ScriptMessageRequestImplRenderer>::FromScriptHandle(
        info.Holder());
  if (!req) {
    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(
          isolate, "Cannot set onreply handler - the underlying object was deleted")));
    return;
  }

  if (value->IsUndefined()) {
    req->SetOnReplyCallback(isolate, v8::Local<v8::Function>());
  } else {
    req->SetOnReplyCallback(isolate, value.As<v8::Function>());
  }

  info.GetReturnValue().Set(value);
}

void ScriptMessageRequestObjectHandler::GetOnError(
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  ScriptMessageRequestImplRenderer* req =
      ScriptReferencedObject<ScriptMessageRequestImplRenderer>::FromScriptHandle(
        info.Holder());
  if (!req) {
    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(
          isolate, "Cannot set onerror handler - the underlying object was deleted")));
    return;
  }

  v8::Handle<v8::Function> callback(req->GetOnErrorCallback(isolate));
  if (callback.IsEmpty()) {
    info.GetReturnValue().Set(v8::Undefined(isolate));
    return;
  }

  info.GetReturnValue().Set(callback);
}

void ScriptMessageRequestObjectHandler::SetOnError(
    v8::Local<v8::Value> value,
    const v8::PropertyCallbackInfo<void>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  if (!value->IsUndefined() && !value->IsFunction()) {
    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(
          isolate, "Unexpected type")));
    return;
  }

  ScriptMessageRequestImplRenderer* req =
      ScriptReferencedObject<ScriptMessageRequestImplRenderer>::FromScriptHandle(
        info.Holder());
  if (!req) {
    return;
  }

  if (value->IsUndefined()) {
    req->SetOnErrorCallback(isolate, v8::Local<v8::Function>());
  } else {
    req->SetOnErrorCallback(isolate, value.As<v8::Function>());
  }

  info.GetReturnValue().Set(value);
}

ScriptMessageRequestObjectHandler::ScriptMessageRequestObjectHandler(
    ScriptMessageManager* mm) :
    ObjectBackedNativeHandler(mm) {
  RouteAccessor(
      "onreply",
      base::Bind(&ScriptMessageRequestObjectHandler::GetOnReply,
                 base::Unretained(this)),
      base::Bind(&ScriptMessageRequestObjectHandler::SetOnReply,
                 base::Unretained(this)),
      v8::DontDelete);
  RouteAccessor(
      "onerror",
      base::Bind(&ScriptMessageRequestObjectHandler::GetOnError,
                 base::Unretained(this)),
      base::Bind(&ScriptMessageRequestObjectHandler::SetOnError,
                 base::Unretained(this)),
      v8::DontDelete);
}

} // namespace oxide
