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

#define V8_ALLOW_ACCESS_TO_RAW_HANDLE_CONSTRUCTOR

#include "oxide_script_message_manager.h"

#include <utility>

#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/child/v8_value_converter.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"
#include "ui/base/resource/resource_bundle.h"

#include "shared/common/oxide_constants.h"
#include "shared/common/oxide_messages.h"

#include "oxide_isolated_world_map.h"
#include "oxide_script_message_handler_renderer.h"
#include "oxide_script_message_request_impl_renderer.h"

#include "grit/oxide_resources.h"

namespace oxide {

namespace {

class StringResource : public v8::String::ExternalOneByteStringResource {
 public:
  StringResource(const base::StringPiece& string) :
      string_(string) {}

  virtual const char* data() const {
    return string_.data();
  }

  virtual size_t length() const {
    return string_.size();
  }

 private:
  base::StringPiece string_;
};

} // namespace

// static
std::string ScriptMessageManager::V8StringToStdString(
    v8::Local<v8::String> string) {
  v8::String::Value v(string);
  base::string16 s(static_cast<const base::char16 *>(*v), v.length());
  return base::UTF16ToUTF8(s);
}

// static
ScriptMessageManager* ScriptMessageManager::GetMessageManagerFromArgs(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::HandleScope handle_scope(args.GetIsolate());

  v8::Handle<v8::External> mm(args.Data().As<v8::External>());
  if (mm.IsEmpty() || mm->IsUndefined()) {
    return nullptr;
  }

  return static_cast<ScriptMessageManager *>(mm->Value());
}

// static
void ScriptMessageManager::OxideLazyGetter(
    v8::Local<v8::String> property,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::HandleScope handle_scope(info.GetIsolate());

  v8::Handle<v8::External> mm(info.Data().As<v8::External>());
  if (mm.IsEmpty() || mm->IsUndefined()) {
    return;
  }

  static_cast<ScriptMessageManager *>(
      mm->Value())->OxideLazyGetterInner(property, info);
}

v8::Handle<v8::Object> ScriptMessageManager::GetOxideApiObject(
      v8::Isolate* isolate) {
  v8::EscapableHandleScope handle_scope(isolate);

  base::StringPiece raw_src(
      ui::ResourceBundle::GetSharedInstance().GetRawDataResource(
        IDR_OXIDE_SCRIPT_MESSAGE_MANAGER_BINDINGS_JS));
  v8::Local<v8::String> src(
      v8::String::NewExternal(isolate, new StringResource(raw_src)));
  DCHECK(!src.IsEmpty() && src->Length() > 0);

  v8::Local<v8::String> start(v8::String::NewFromUtf8(
      isolate,
      "(function(sendMessageNative, addMessageHandlerNative, removeMessageHandlerNative, exports) {\n"
  ));
  v8::Local<v8::String> end(v8::String::NewFromUtf8(isolate, "\n})"));
  v8::Local<v8::String> wrapped_src(
      v8::String::Concat(start, v8::String::Concat(src, end)));

  v8::Local<v8::Script> script(v8::Script::Compile(wrapped_src));

  v8::MicrotasksScope microstasks(isolate,
                                  v8::MicrotasksScope::kDoNotRunMicrotasks);

  v8::TryCatch try_catch;
  v8::Local<v8::Function> function(script->Run().As<v8::Function>());
  if (try_catch.HasCaught()) {
    LOG(ERROR) << "Caught exception when running script";
    return v8::Handle<v8::Object>();
  }

  v8::Local<v8::External> local_data(closure_data_.NewHandle(isolate));

  v8::Local<v8::FunctionTemplate> send_message_template(
      v8::FunctionTemplate::New(isolate, SendMessage, local_data));
  v8::Local<v8::FunctionTemplate> add_message_handler_template(
      v8::FunctionTemplate::New(isolate, AddMessageHandler, local_data));
  v8::Local<v8::FunctionTemplate> remove_message_handler_template(
      v8::FunctionTemplate::New(isolate, RemoveMessageHandler, local_data));

  v8::Local<v8::Object> exports(v8::Object::New(isolate));

  v8::Local<v8::Value> argv[] = {
    send_message_template->GetFunction(),
    add_message_handler_template->GetFunction(),
    remove_message_handler_template->GetFunction(),
    exports
  };

  frame_->GetWebFrame()->callFunctionEvenIfScriptDisabled(
      function,
      GetV8Context()->Global(),
      arraysize(argv),
      argv);

  if (try_catch.HasCaught()) {
    LOG(ERROR) << "Caught exception when running script function";
    return v8::Handle<v8::Object>();
  }

  return handle_scope.Escape(exports);
}

void ScriptMessageManager::OxideLazyGetterInner(
    v8::Local<v8::String> property,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  v8::Handle<v8::Object> exports(GetOxideApiObject(isolate));

  v8::Handle<v8::Value> val = info.This();
  if (val->IsObject()) {
    v8::Handle<v8::Object> object = v8::Handle<v8::Object>::Cast(val);
    object->Delete(property);
    object->Set(property, exports);
  } else {
    NOTREACHED();
  }
  info.GetReturnValue().Set(exports);
}

// static
void ScriptMessageManager::SendMessage(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  GetMessageManagerFromArgs(args)->SendMessageInner(args);
}

void ScriptMessageManager::SendMessageInner(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  DCHECK_EQ(isolate, this->isolate());
  v8::HandleScope handle_scope(isolate);

  DCHECK(args.Length() == 3);

  v8::Local<v8::Value> msg_want_reply_as_val = args[0];
  v8::Local<v8::Value> msg_id_as_val = args[1];
  v8::Local<v8::Value> msg_payload = args[2];

  DCHECK(msg_want_reply_as_val->IsBoolean());
  if (!msg_id_as_val->IsString()) {
    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(
          isolate, "Invalid message ID")));
    return;
  }

  v8::Local<v8::Boolean> msg_want_reply = msg_want_reply_as_val->ToBoolean();
  v8::Local<v8::String> msg_id = msg_id_as_val.As<v8::String>();

  scoped_ptr<content::V8ValueConverter> converter(
      content::V8ValueConverter::create());
  scoped_ptr<base::Value> payload(
      converter->FromV8Value(msg_payload, isolate->GetCallingContext()));

  v8::Handle<v8::Object> handle;
  int serial = ScriptMessageParams::kInvalidSerial;

  if (msg_want_reply->Value()) {
    handle = script_message_request_object_handler_.NewInstance();

    scoped_refptr<ScriptMessageRequestImplRenderer> req =
        new ScriptMessageRequestImplRenderer(
          this, next_message_id_++, handle);
    serial = req->serial();
  }

  ScriptMessageParams params;
  PopulateScriptMessageParams(serial,
                              GetContextURL(),
                              V8StringToStdString(msg_id),
                              std::move(payload),
                              &params);

  if (!frame()->Send(new OxideHostMsg_SendMessage(frame()->GetRoutingID(),
                                                  params))) {
    return;
  }

  if (msg_want_reply->Value()) {
    args.GetReturnValue().Set(handle);
  }
}

// static
void ScriptMessageManager::AddMessageHandler(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  GetMessageManagerFromArgs(args)->AddMessageHandlerInner(args);
}

void ScriptMessageManager::AddMessageHandlerInner(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  if (args.Length() < 2) {
    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(
          isolate, "Insufficient number of arguments")));
    return;
  }

  v8::Local<v8::Value> msg_id_as_val = args[0];
  v8::Local<v8::Value> msg_handler_as_val = args[1];

  if (!msg_id_as_val->IsString()) {
    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(
          isolate, "Invalid message ID")));
    return;
  }
  if (!msg_handler_as_val->IsFunction()) {
    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(
          isolate, "Invalid handler callback")));
    return;
  }

  v8::Local<v8::String> msg_id = msg_id_as_val.As<v8::String>();
  v8::Local<v8::Function> msg_handler = msg_handler_as_val.As<v8::Function>();

  std::string s_msg_id = V8StringToStdString(msg_id);

  if (script_message_handler_map_.find(s_msg_id) !=
      script_message_handler_map_.end()) {
    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(
          isolate, "Can't register more than one handler for the same message")));
    return;
  }

  ScriptMessageHandlerRenderer* handler =
      new ScriptMessageHandlerRenderer(this, s_msg_id, msg_handler);
  script_message_handler_map_[s_msg_id] =
      linked_ptr<ScriptMessageHandlerRenderer>(handler);
}

// static
void ScriptMessageManager::RemoveMessageHandler(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  GetMessageManagerFromArgs(args)->RemoveMessageHandlerInner(args);
}

void ScriptMessageManager::RemoveMessageHandlerInner(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  if (args.Length() < 1) {
    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(
          isolate, "Insufficient number of arguments")));
    return;
  }

  v8::Local<v8::Value> msg_id_as_val = args[0];

  if (!msg_id_as_val->IsString()) {
    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(
          isolate, "Invalid message ID")));
    return;
  }

  v8::Local<v8::String> msg_id = msg_id_as_val.As<v8::String>();
  std::string s_msg_id = V8StringToStdString(msg_id);

  ScriptMessageHandlerMap::iterator it = script_message_handler_map_.find(s_msg_id);
  if (it == script_message_handler_map_.end()) {
    isolate->ThrowException(v8::Exception::Error(
        v8::String::NewFromUtf8(
          isolate, "No handler is registered for this message ID")));
    return;
  }

  script_message_handler_map_.erase(it);
}

void ScriptMessageManager::AddScriptMessageRequest(
    ScriptMessageRequestImplRenderer* req) {
  for (ScriptMessageRequestVector::iterator it =
        current_script_message_requests_.begin();
       it != current_script_message_requests_.end(); ++it) {
    if ((*it) == req) {
      return;
    }
  }

  current_script_message_requests_.push_back(req);
}

void ScriptMessageManager::RemoveScriptMessageRequest(
    ScriptMessageRequestImplRenderer* req) {
  for (ScriptMessageRequestVector::iterator it =
        current_script_message_requests_.begin();
       it != current_script_message_requests_.end(); ++it) {
    if ((*it) == req) {
      current_script_message_requests_.erase(it);
      return;
    }
  }
}

ScriptMessageManager::ScriptMessageManager(content::RenderFrame* frame,
                                           v8::Handle<v8::Context> context,
                                           int world_id) :
    frame_(frame),
    isolate_(context->GetIsolate()),
    context_(isolate_, context),
    world_id_(world_id),
    next_message_id_(0),
    script_message_request_object_handler_(this),
    script_message_object_handler_(this) {
  v8::HandleScope handle_scope(isolate());
  v8::Context::Scope context_scope(context);

  closure_data_.reset(isolate(), v8::External::New(isolate(), this));
  v8::Local<v8::External> local_data(closure_data_.NewHandle(isolate()));

  if (world_id_ != kMainWorldId) {
    v8::Local<v8::Object> global(context->Global());
    global->Set(v8::String::NewFromUtf8(isolate(), "oxide"),
                GetOxideApiObject(isolate()));
    // XXX(chrisccoulson): Started failing between Chromium 45.0.2431.0 and
    //  45.0.2433.0
    //global->SetAccessor(v8::String::NewFromUtf8(isolate(), "oxide"),
    //                    OxideLazyGetter, nullptr,
    //                    local_data);
  }
}

ScriptMessageManager::~ScriptMessageManager() {}

v8::Handle<v8::Context> ScriptMessageManager::GetV8Context() const {
  return context_.NewHandle(isolate());
}

GURL ScriptMessageManager::GetContextURL() const {
  return IsolatedWorldMap::URLFromID(world_id_);
}

ScriptMessageHandlerRenderer* ScriptMessageManager::GetHandlerForMsgID(
    const std::string& msg_id) {
  ScriptMessageHandlerMap::iterator it = script_message_handler_map_.find(msg_id);
  if (it == script_message_handler_map_.end()) {
    return nullptr;
  }

  return it->second.get();
}

} // namespace oxide
