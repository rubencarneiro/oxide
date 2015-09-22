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

#include "oxide_script_message_object_handler.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/child/v8_value_converter.h"
#include "third_party/WebKit/public/web/WebScopedMicrotaskSuppression.h"

#include "shared/common/oxide_script_message_request.h"

#include "oxide_script_message_impl_renderer.h"
#include "oxide_script_message_manager.h"
#include "oxide_script_referenced_object.h"

namespace oxide {

void ScriptMessageObjectHandler::Reply(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  ScriptMessageImplRenderer* message =
      ScriptReferencedObject<ScriptMessageImplRenderer>::FromScriptHandle(
        info.Holder());
  if (!message) {
    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(
          isolate, "Failed to send reply - the underlying object was deleted")));
    return;
  }

  v8::Local<v8::Value> payload;
  if (info.Length() > 0) {
    payload = info[0];
  }

  scoped_ptr<content::V8ValueConverter> converter(
      content::V8ValueConverter::create());
  message->Reply(
      make_scoped_ptr(converter->FromV8Value(payload,
                                             isolate->GetCallingContext())));
}

void ScriptMessageObjectHandler::Error(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  ScriptMessageImplRenderer* message =
      ScriptReferencedObject<ScriptMessageImplRenderer>::FromScriptHandle(
        info.Holder());
  if (!message) {
    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(
          isolate, "Failed to send error - the underlying object was deleted")));
    return;
  }

  if (info.Length() == 0) {
    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(
          isolate, "Unexpected number of arguments")));
    return;
  }

  scoped_ptr<content::V8ValueConverter> converter(
      content::V8ValueConverter::create());
  message->Error(
      ScriptMessageParams::ERROR_HANDLER_REPORTED_ERROR,
      make_scoped_ptr(converter->FromV8Value(info[0],
                                             isolate->GetCallingContext())));
}

void ScriptMessageObjectHandler::GetID(
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  ScriptMessageImplRenderer* message =
      ScriptReferencedObject<ScriptMessageImplRenderer>::FromScriptHandle(
        info.Holder());
  if (!message) {
    return;
  }

  info.GetReturnValue().Set(
      v8::String::NewFromUtf8(isolate, message->msg_id().c_str()));
}

void ScriptMessageObjectHandler::GetPayload(
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  ScriptMessageImplRenderer* message =
      ScriptReferencedObject<ScriptMessageImplRenderer>::FromScriptHandle(
        info.Holder());
  if (!message) {
    return;
  }

  scoped_ptr<content::V8ValueConverter> converter(
      content::V8ValueConverter::create());
  info.GetReturnValue().Set(
      converter->ToV8Value(message->payload(), isolate->GetCallingContext()));
}

ScriptMessageObjectHandler::ScriptMessageObjectHandler(
    ScriptMessageManager* mm)
    : ObjectBackedNativeHandler(mm) {
  RouteAccessor("id",
                base::Bind(&ScriptMessageObjectHandler::GetID,
                           base::Unretained(this)),
                HandlerSetter(),
                v8::DontDelete);
  RouteAccessor("args",
                base::Bind(&ScriptMessageObjectHandler::GetPayload,
                           base::Unretained(this)),
                HandlerSetter(),
                v8::DontDelete);
  RouteAccessor("payload",
                base::Bind(&ScriptMessageObjectHandler::GetPayload,
                           base::Unretained(this)),
                HandlerSetter(),
                v8::DontDelete);
  RouteFunction("reply",
                base::Bind(&ScriptMessageObjectHandler::Reply,
                           base::Unretained(this)));
  RouteFunction("error",
                base::Bind(&ScriptMessageObjectHandler::Error,
                           base::Unretained(this)));
}

} // namespace oxide
