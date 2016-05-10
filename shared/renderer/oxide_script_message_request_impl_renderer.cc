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

#include "oxide_script_message_request_impl_renderer.h"

#include <memory>

#include "base/logging.h"
#include "content/public/child/v8_value_converter.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"

#include "shared/common/oxide_messages.h"

#include "oxide_script_message_manager.h"

namespace oxide {

ScriptMessageRequestImplRenderer::~ScriptMessageRequestImplRenderer() {
  if (manager()) {
    manager()->RemoveScriptMessageRequest(this);
  }
}

void ScriptMessageRequestImplRenderer::DispatchResponse(
    v8::Handle<v8::Function> function,
    int argc,
    v8::Local<v8::Value> argv[]) {
  v8::TryCatch try_catch(manager()->isolate());
  {
    v8::MicrotasksScope microtasks(manager()->isolate(),
                                   v8::MicrotasksScope::kDoNotRunMicrotasks);
    manager()->frame()->GetWebFrame()->callFunctionEvenIfScriptDisabled(
        function, GetHandle(), argc, argv);
  }
  if (try_catch.HasCaught()) {
    LOG(WARNING) << "Response handler threw an exception";
  }
}

void ScriptMessageRequestImplRenderer::OnReply(const base::Value& payload) {
  v8::Isolate* isolate = manager()->isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Context::Scope context_scope(manager()->GetV8Context());

  v8::Handle<v8::Function> callback(reply_callback_.NewHandle(isolate));
  if (callback.IsEmpty()) {
    return;
  }

  std::unique_ptr<content::V8ValueConverter> converter(
      content::V8ValueConverter::create());

  v8::Local<v8::Value> argv[] = {
    converter->ToV8Value(&payload, manager()->GetV8Context())
  };

  DispatchResponse(callback, arraysize(argv), argv);
}

void ScriptMessageRequestImplRenderer::OnError(
    ScriptMessageParams::Error error,
    const base::Value& payload) {
  v8::Isolate* isolate = manager()->isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Context::Scope context_scope(manager()->GetV8Context());

  v8::Handle<v8::Function> callback(error_callback_.NewHandle(isolate));
  if (callback.IsEmpty()) {
    return;
  }

  std::unique_ptr<content::V8ValueConverter> converter(
      content::V8ValueConverter::create());

  v8::Local<v8::Value> argv[] = {
    v8::Integer::New(isolate, int(error)),
    converter->ToV8Value(&payload, manager()->GetV8Context())
  };

  DispatchResponse(callback, arraysize(argv), argv);
}

ScriptMessageRequestImplRenderer::ScriptMessageRequestImplRenderer(
    ScriptMessageManager* mm,
    int serial,
    const v8::Handle<v8::Object>& handle)
    : ScriptMessageRequest(serial),
      ScriptReferencedObject<ScriptMessageRequestImplRenderer>(mm, handle) {
  manager()->AddScriptMessageRequest(this);
}

v8::Handle<v8::Function>
ScriptMessageRequestImplRenderer::GetOnReplyCallback(
    v8::Isolate* isolate) const {
  return reply_callback_.NewHandle(isolate);
}

void ScriptMessageRequestImplRenderer::SetOnReplyCallback(
    v8::Isolate* isolate,
    v8::Handle<v8::Function> callback) {
  reply_callback_.reset(isolate, callback);
}

v8::Handle<v8::Function>
ScriptMessageRequestImplRenderer::GetOnErrorCallback(
    v8::Isolate* isolate) const {
  return error_callback_.NewHandle(isolate);
}

void ScriptMessageRequestImplRenderer::SetOnErrorCallback(
    v8::Isolate* isolate,
    v8::Handle<v8::Function> callback) {
  error_callback_.reset(isolate, callback);
}

} // namespace oxide
