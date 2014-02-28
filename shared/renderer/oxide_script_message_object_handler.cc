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

#include "oxide_script_message_object_handler.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "third_party/WebKit/public/web/WebScopedMicrotaskSuppression.h"

#include "shared/common/oxide_script_message_request.h"

#include "oxide_script_message_impl_renderer.h"
#include "oxide_script_message_manager.h"
#include "oxide_script_owned_object.h"

namespace oxide {

namespace {

std::string V8StringToStdString(
    v8::Local<v8::String> string) {
  v8::String::Value v(string);
  base::string16 s(static_cast<const base::char16 *>(*v), v.length());
  return base::UTF16ToUTF8(s);
}

}

void ScriptMessageObjectHandler::Reply(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  ScriptMessageImplRenderer* message =
      static_cast<ScriptMessageImplRenderer *>(
        ScriptOwnedObject::FromScriptHandle(info.This()));
  if (!message) {
    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(
          isolate, "Failed to send reply - the underlying object was deleted")));
    return;
  }

  v8::Handle<v8::Object> args;
  if (info.Length() > 0) {
    if (!info[0]->IsObject() && !info[0]->IsUndefined() && !info[0]->IsNull()) {
      isolate->ThrowException(v8::Exception::Error(
          v8::String::NewFromUtf8(
            isolate, "Unexpected argument type")));
      return;
    }
    if (info[0]->IsObject()) {
      args = info[0].As<v8::Object>();
    }
  }
  if (args.IsEmpty()) {
    args = v8::Object::New(isolate);
  }

  v8::TryCatch try_catch;
  v8::Handle<v8::String> str_args(Stringify(isolate, args));
  if (str_args.IsEmpty() || try_catch.HasCaught()) {
    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(
          isolate, "Failed to serialize arguments")));
    return;
  }

  message->Reply(V8StringToStdString(str_args));
}

void ScriptMessageObjectHandler::Error(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  ScriptMessageImplRenderer* message =
      static_cast<ScriptMessageImplRenderer *>(
        ScriptOwnedObject::FromScriptHandle(info.This()));
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

  v8::Handle<v8::String> msg(info[0]->ToString());

  message->Error(ScriptMessageRequest::ERROR_HANDLER_REPORTED_ERROR,
                 V8StringToStdString(msg));
}

void ScriptMessageObjectHandler::GetID(
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  ScriptMessageImplRenderer* message =
      static_cast<ScriptMessageImplRenderer *>(
        ScriptOwnedObject::FromScriptHandle(info.Holder()));
  if (!message) {
    return;
  }

  info.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, message->msg_id().c_str()));
}

void ScriptMessageObjectHandler::GetArgs(
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  ScriptMessageImplRenderer* message =
      static_cast<ScriptMessageImplRenderer *>(
        ScriptOwnedObject::FromScriptHandle(info.This()));
  if (!message) {
    return;
  }

  info.GetReturnValue().Set(
      v8::JSON::Parse(v8::String::NewFromUtf8(isolate, message->args().c_str())));
}

v8::Handle<v8::String> ScriptMessageObjectHandler::Stringify(
    v8::Isolate* isolate,
    v8::Handle<v8::Object> object) {
  v8::EscapableHandleScope handle_scope(isolate);

  v8::Handle<v8::Function> func(stringify_func_.NewHandle(isolate));

  v8::Handle<v8::Value> argv[] = { object };
  v8::Local<v8::Value> res = func->Call(object->CreationContext()->Global(),
                                        arraysize(argv),
                                        argv);
  if (!res->IsString()) {
    return v8::Handle<v8::String>();
  }

  return handle_scope.Escape(res.As<v8::String>());
}

ScriptMessageObjectHandler::ScriptMessageObjectHandler(
    ScriptMessageManager* mm) :
    ObjectBackedNativeHandler(mm) {
  v8::Isolate* isolate = mm->isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Context::Scope context_scope(mm->GetV8Context());

  v8::Local<v8::String> stringify_src(v8::String::NewFromUtf8(
      isolate,
      "(function(o) { return JSON.stringify(o); })"));
  v8::TryCatch try_catch;
  v8::Local<v8::Script> stringify_script(v8::Script::New(stringify_src));
  {
    blink::WebScopedMicrotaskSuppression mts;
    stringify_func_.reset(isolate, stringify_script->Run().As<v8::Function>());
  }
  DCHECK(!try_catch.HasCaught());

  RouteAccessor("id",
                base::Bind(&ScriptMessageObjectHandler::GetID,
                           base::Unretained(this)),
                HandlerSetter(),
                v8::DontDelete);
  RouteAccessor("args",
                base::Bind(&ScriptMessageObjectHandler::GetArgs,
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
