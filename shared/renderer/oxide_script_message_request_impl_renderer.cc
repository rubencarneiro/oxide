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

#include "oxide_script_message_request_impl_renderer.h"

#include "base/logging.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebScopedMicrotaskSuppression.h"

#include "shared/common/oxide_messages.h"

#include "oxide_script_message_manager.h"

namespace oxide {

ScriptMessageRequestImplRenderer::~ScriptMessageRequestImplRenderer() {
  if (manager()) {
    manager()->RemoveScriptMessageRequest(this);
  }
}

bool ScriptMessageRequestImplRenderer::DoSendMessage(
    const OxideMsg_SendMessage_Params& params) {
  content::RenderFrame* frame = manager()->frame();
  return frame->Send(new OxideHostMsg_SendMessage(
      frame->GetRoutingID(), params));
}

void ScriptMessageRequestImplRenderer::OnReply(const std::string& args) {
  v8::Isolate* isolate = manager()->isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Context::Scope context_scope(manager()->GetV8Context());

  v8::Handle<v8::Function> callback(reply_callback_.NewHandle(isolate));
  if (callback.IsEmpty()) {
    return;
  }

  v8::Handle<v8::Value> argv[] = {
    v8::JSON::Parse(v8::String::NewFromUtf8(isolate, args.c_str()))
  };

  DispatchResponse(callback, arraysize(argv), argv);
}

void ScriptMessageRequestImplRenderer::OnError(
    ScriptMessageRequest::Error error,
    const std::string& msg) {
  v8::Isolate* isolate = manager()->isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Context::Scope context_scope(manager()->GetV8Context());

  v8::Handle<v8::Function> callback(error_callback_.NewHandle(isolate));
  if (callback.IsEmpty()) {
    return;
  }

  v8::Handle<v8::Value> argv[] = {
    v8::Integer::New(isolate, int(error)),
    v8::String::NewFromUtf8(isolate, msg.c_str())
  };

  DispatchResponse(callback, arraysize(argv), argv);
}

void ScriptMessageRequestImplRenderer::DispatchResponse(
    v8::Handle<v8::Function> function,
    int argc,
    v8::Handle<v8::Value> argv[]) {
  v8::TryCatch try_catch;
  {
    blink::WebScopedMicrotaskSuppression mts;
    manager()->frame()->GetWebFrame()->callFunctionEvenIfScriptDisabled(
        function, GetHandle(), argc, argv);
  }
  if (try_catch.HasCaught()) {
    LOG(WARNING) << "Response handler threw an exception";
  }
}

ScriptMessageRequestImplRenderer::ScriptMessageRequestImplRenderer(
    ScriptMessageManager* mm,
    int serial,
    bool want_reply,
    const std::string& msg_id,
    const std::string& args,
    const v8::Handle<v8::Object>& handle) :
    ScriptMessageRequest(serial, mm->GetContextURL(), want_reply, msg_id, args),
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
