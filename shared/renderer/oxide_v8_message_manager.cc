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

#include "oxide_v8_message_manager.h"

#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebScopedMicrotaskSuppression.h"
#include "ui/base/resource/resource_bundle.h"

#include "shared/common/oxide_messages.h"

#include "oxide_isolated_world_map.h"

#include "grit/oxide_resources.h"

namespace oxide {

namespace {

class StringResource : public v8::String::ExternalAsciiStringResource {
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
std::string V8MessageManager::V8StringToStdString(
    v8::Local<v8::String> string) {
  v8::String::Value v(string);
  base::string16 s(static_cast<const char16 *>(*v), v.length());
  return base::UTF16ToUTF8(s);
}

// static
V8MessageManager* V8MessageManager::GetMessageManagerFromArgs(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::HandleScope handle_scope(v8::Isolate::GetCurrent());

  v8::Handle<v8::External> mm(args.Data().As<v8::External>());
  if (mm.IsEmpty() || mm->IsUndefined()) {
    return NULL;
  }

  return static_cast<V8MessageManager *>(mm->Value());
}

// static
void V8MessageManager::SendMessage(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  GetMessageManagerFromArgs(args)->SendMessageInner(args);
}

// static
void V8MessageManager::RegisterReceiveHandler(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  GetMessageManagerFromArgs(args)->RegisterReceiveHandlerInner(args);
}

void V8MessageManager::SendMessageInner(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::HandleScope handle_scope(v8::Isolate::GetCurrent());

  if (args.Length() < 3) {
    v8::ThrowException(v8::Exception::Error(v8::String::New(
        "Insufficient arguments")));
    return;
  }

  v8::Local<v8::Value> serial_as_val = args[0];
  v8::Local<v8::Value> type_as_val = args[1];
  v8::Local<v8::Value> msg_id_as_val = args[2];

  if (!serial_as_val->IsInt32() || !type_as_val->IsInt32()) {
    v8::ThrowException(v8::Exception::Error(v8::String::New(
        "Unexpected argument types")));
    return;
  }

  v8::Local<v8::Int32> serial = serial_as_val->ToInt32();

  if (!OxideMsg_SendMessage_Type::is_valid(type_as_val->ToInt32()->Value())) {
    v8::ThrowException(v8::Exception::Error(v8::String::New(
        "Message type value out of range")));
    return;
  }

  if (!msg_id_as_val->IsString()) {
    v8::ThrowException(v8::Exception::Error(v8::String::New(
        "Invalid message ID")));
    return;
  }

  OxideMsg_SendMessage_Type::Value type =
      static_cast<OxideMsg_SendMessage_Type::Value>(
        type_as_val->ToInt32()->Value());

  v8::Local<v8::String> msg_id = msg_id_as_val->ToString();

  v8::Local<v8::String> msg_args;
  if (args.Length() > 3) {
    v8::Local<v8::Value> msg_args_as_val = args[3];
    if (!msg_args_as_val->IsString()) {
      v8::ThrowException(v8::Exception::Error(v8::String::New(
          "Invalid argument type")));
      return;
    }

    msg_args = msg_args_as_val->ToString();
  } else if (type == OxideMsg_SendMessage_Type::Error) {
    msg_args = v8::String::Empty();
  } else {
    msg_args = v8::String::New("{}");
  }

  OxideMsg_SendMessage_Params params;
  params.frame_id = frame_->identifier();
  params.world_id = IsolatedWorldMap::IDToName(world_id_);
  params.serial = serial->Value();
  params.type = type;
  params.msg_id = V8StringToStdString(msg_id);
  params.args = V8StringToStdString(msg_args);

  content::RenderThread::Get()->Send(
      new OxideHostMsg_SendMessage(render_view()->GetRoutingID(),
                                   params));
}

void V8MessageManager::RegisterReceiveHandlerInner(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::HandleScope handle_scope(v8::Isolate::GetCurrent());

  if (args.Length() < 1) {
    v8::ThrowException(v8::Exception::Error(v8::String::New(
        "Insufficient arguments")));
    return;
  }

  v8::Local<v8::Value> handler_as_val = args[0];
  if (!handler_as_val->IsFunction()) {
    v8::ThrowException(v8::Exception::Error(v8::String::New(
        "Handler must be a function")));
    return;
  }

  receive_handler_.reset(handler_as_val.As<v8::Function>());
}

// static
void V8MessageManager::OxideLazyGetter(
    v8::Local<v8::String> property,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::HandleScope handle_scope(v8::Isolate::GetCurrent());

  v8::Handle<v8::External> mm(info.Data().As<v8::External>());
  if (mm.IsEmpty() || mm->IsUndefined()) {
    return;
  }

  static_cast<V8MessageManager *>(mm->Value())->OxideLazyGetterInner(property,
                                                                     info);
}

void V8MessageManager::OxideLazyGetterInner(
    v8::Local<v8::String> property,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  base::StringPiece raw_src(
      ui::ResourceBundle::GetSharedInstance().GetRawDataResource(
        IDR_OXIDE_V8_MESSAGE_MANAGER_BINDINGS_JS));
  v8::Local<v8::String> src(
      v8::String::NewExternal(new StringResource(raw_src)));
  if (src.IsEmpty() || src->Length() == 0) {
    LOG(ERROR) << "Empty source";
    return;
  }

  v8::Local<v8::String> start(v8::String::New(
    "(function(sendMessageNative, registerReceiveHandlerNative, exports) {\n"
  ));
  v8::Local<v8::String> end(v8::String::New("\n})"));
  v8::Local<v8::String> wrapped_src(
      v8::String::Concat(start, v8::String::Concat(src, end)));

  v8::Local<v8::Script> script(v8::Script::New(wrapped_src));

  v8::TryCatch try_catch;
  v8::Local<v8::Function> function(script->Run().As<v8::Function>());
  if (try_catch.HasCaught()) {
    LOG(ERROR) << "Caught exception when running script";
    return;
  }

  v8::Local<v8::External> local_data(closure_data_.get());

  v8::Local<v8::FunctionTemplate> send_message_template(
      v8::FunctionTemplate::New(SendMessage, local_data));
  v8::Local<v8::FunctionTemplate> register_receive_handler_template(
      v8::FunctionTemplate::New(RegisterReceiveHandler, local_data));

  v8::Local<v8::Object> exports(v8::Object::New());

  v8::Handle<v8::Value> args[] = {
    send_message_template->GetFunction(),
    register_receive_handler_template->GetFunction(),
    exports
  };

  {
    WebKit::WebScopedMicrotaskSuppression mts;
    frame_->callFunctionEvenIfScriptDisabled(function,
                                             context_->Global(),
                                             arraysize(args),
                                             args);
  }
  if (try_catch.HasCaught()) {
    LOG(ERROR) << "Caught exception when running script function";
    return;
  }

  info.This()->Delete(property);
  info.This()->Set(property, exports);
  info.GetReturnValue().Set(exports);
}

V8MessageManager::V8MessageManager(WebKit::WebFrame* frame,
                                   v8::Handle<v8::Context> context,
                                   int world_id) :
    frame_(frame),
    context_(context),
    world_id_(world_id),
    closure_data_(v8::External::New(this)) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Context::Scope context_scope(context);

  v8::Local<v8::External> local_data(closure_data_.get());

  v8::Local<v8::Object> global(context->Global());
  global->SetAccessor(v8::String::New("oxide"),
                      OxideLazyGetter, NULL,
                      local_data);
}

V8MessageManager::~V8MessageManager() {}

void V8MessageManager::ReceiveMessage(
    const OxideMsg_SendMessage_Params& params) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Context::Scope context_scope(context_.get());

  CHECK(!receive_handler_.get().IsEmpty());
  CHECK(OxideMsg_SendMessage_Type::is_valid(params.type));

  v8::Handle<v8::Value> args[] = {
    v8::Integer::New(params.serial),
    v8::Integer::New(params.type),
    v8::String::New(params.msg_id.data()),
    v8::String::New(params.args.data())
  };

  v8::TryCatch try_catch;
  frame_->callFunctionEvenIfScriptDisabled(receive_handler_.get(),
                                           context_->Global(),
                                           arraysize(args),
                                           args);
  if (try_catch.HasCaught() &&
      params.type == OxideMsg_SendMessage_Type::Message) {
    OxideMsg_SendMessage_Params error_params;
    error_params.frame_id = params.frame_id;
    error_params.world_id = params.world_id;
    error_params.serial = params.serial;
    error_params.type = OxideMsg_SendMessage_Type::Error;
    error_params.msg_id = params.msg_id;
    error_params.args = std::string("Handler threw an exception");

    content::RenderThread::Get()->Send(
        new OxideHostMsg_SendMessage(render_view()->GetRoutingID(),
                                     error_params));
  }
}

v8::Handle<v8::Context> V8MessageManager::v8_context() const {
  return context_.get();
}

long long V8MessageManager::frame_id() const {
  return frame_->identifier();
}

content::RenderView* V8MessageManager::render_view() const {
  return content::RenderView::FromWebView(frame_->view());
}

} // namespace oxide
