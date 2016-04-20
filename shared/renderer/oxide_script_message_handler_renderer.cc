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

#include "oxide_script_message_handler_renderer.h"

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"

#include "oxide_script_message_impl_renderer.h"
#include "oxide_script_message_manager.h"

namespace oxide {

namespace {

std::unique_ptr<base::StringValue> V8StringToValue(
    v8::Local<v8::String> string) {
  v8::String::Value v(string);
  base::string16 s(static_cast<const base::char16 *>(*v), v.length());
  return base::WrapUnique(new base::StringValue(base::UTF16ToUTF8(s)));
}

}

bool ScriptMessageHandlerRenderer::ReceiveMessageCallback(
    ScriptMessage* message,
    std::unique_ptr<base::Value>* error_payload) {
  v8::HandleScope handle_scope(manager_->isolate());
  v8::Context::Scope context_scope(manager_->GetV8Context());

  v8::Handle<v8::Function> function(callback_.NewHandle(manager_->isolate()));

  ScriptMessageImplRenderer* m =
      static_cast<ScriptMessageImplRenderer *>(message);

  v8::Local<v8::Value> argv[] = {
    m->GetHandle()
  };

  v8::TryCatch try_catch;
  {
    v8::MicrotasksScope microtasks(manager_->isolate(),
                                   v8::MicrotasksScope::kDoNotRunMicrotasks);
    manager_->frame()->GetWebFrame()->callFunctionEvenIfScriptDisabled(
        function, manager_->GetV8Context()->Global(), arraysize(argv), argv);
  }

  if (try_catch.HasCaught()) {
    // In this case, ownership of ScriptMessage is retained by our caller
    // (ScriptMessagehandler::OnReceiveMessage), and the JS handle is kept
    // alive by its caller (ScriptMessageDispatcherRenderer::OnReceiveMessage).
    // Our caller will dispose of ScriptMessage, before the current V8 handle
    // scope exists
    *error_payload = V8StringToValue(try_catch.Message()->Get());
    return false;
  }

  return true;
}

ScriptMessageHandlerRenderer::ScriptMessageHandlerRenderer(
    ScriptMessageManager* mm,
    const std::string& msg_id,
    const v8::Handle<v8::Function>& callback) :
    manager_(mm),
    callback_(mm->isolate(), callback) {
  handler_.set_msg_id(msg_id);
  handler_.SetCallback(
      base::Bind(&ScriptMessageHandlerRenderer::ReceiveMessageCallback,
                 // The callback cannot run after |this| is deleted, as it
                 // exclusively owns |handler_|
                 base::Unretained(this)));
}

} // namespace oxide
