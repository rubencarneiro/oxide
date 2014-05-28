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

#include "oxide_object_backed_native_handler.h"

#include "base/logging.h"

#include "oxide_script_message_manager.h"

namespace oxide {

namespace {

const char kHandlerFunction[] = "handler_function";
const char kHandlerGetter[] = "handler_getter";
const char kHandlerSetter[] = "handler_setter";
const char kPropertyName[] = "property_name";

}

// static
void ObjectBackedNativeHandler::FunctionRouter(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Handle<v8::Object> data = args.Data().As<v8::Object>();

  v8::Handle<v8::Value> handler_function_value =
      data->Get(v8::String::NewFromUtf8(isolate, kHandlerFunction));
  if (handler_function_value.IsEmpty() ||
      handler_function_value->IsUndefined()) {
    return;
  }
  DCHECK(handler_function_value->IsExternal());

  static_cast<HandlerFunction*>(
      handler_function_value.As<v8::External>()->Value())->Run(args);
}

// static
void ObjectBackedNativeHandler::GetterRouter(
    v8::Local<v8::String> property,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Handle<v8::Object> data = info.Data().As<v8::Object>();

  v8::Handle<v8::Value> handler_property_name_value =
      data->Get(v8::String::NewFromUtf8(isolate, kPropertyName));
  if (handler_property_name_value.IsEmpty() ||
      handler_property_name_value->IsUndefined()) {
    return;
  }
  DCHECK(handler_property_name_value->IsString());
  DCHECK(handler_property_name_value.As<v8::String>()->Equals(property));

  v8::Handle<v8::Value> handler_getter_value =
      data->Get(v8::String::NewFromUtf8(isolate, kHandlerGetter));
  if (handler_getter_value.IsEmpty() ||
      handler_getter_value->IsUndefined()) {
    return;
  }
  DCHECK(handler_getter_value->IsExternal());

  static_cast<HandlerGetter *>(
      handler_getter_value.As<v8::External>()->Value())->Run(info);
}

// static
void ObjectBackedNativeHandler::SetterRouter(
    v8::Local<v8::String> property,
    v8::Local<v8::Value> value,
    const v8::PropertyCallbackInfo<void>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Handle<v8::Object> data = info.Data().As<v8::Object>();

  v8::Handle<v8::Value> handler_property_name_value =
      data->Get(v8::String::NewFromUtf8(isolate, kPropertyName));
  if (handler_property_name_value.IsEmpty() ||
      handler_property_name_value->IsUndefined()) {
    return;
  }
  DCHECK(handler_property_name_value->IsString());
  DCHECK(handler_property_name_value.As<v8::String>()->Equals(property));

  v8::Handle<v8::Value> handler_setter_value =
      data->Get(v8::String::NewFromUtf8(isolate, kHandlerSetter));
  if (handler_setter_value.IsEmpty() ||
      handler_setter_value->IsUndefined()) {
    return;
  }
  DCHECK(handler_setter_value->IsExternal());

  static_cast<HandlerSetter *>(
      handler_setter_value.As<v8::External>()->Value())->Run(value, info);
}

void ObjectBackedNativeHandler::RouteFunction(
    const std::string& name,
    const HandlerFunction& handler_function) {
  v8::HandleScope handle_scope(manager_->isolate());
  v8::Context::Scope context_scope(manager_->GetV8Context());

  v8::Persistent<v8::Object> data(
      manager_->isolate(), v8::Object::New(manager_->isolate()));
  v8::Local<v8::Object> local_data(
      v8::Local<v8::Object>::New(manager_->isolate(), data));
  local_data->Set(
      v8::String::NewFromUtf8(manager_->isolate(), kHandlerFunction),
      v8::External::New(manager_->isolate(),
                        new HandlerFunction(handler_function)));
  v8::Handle<v8::FunctionTemplate> function_template(
      v8::FunctionTemplate::New(manager_->isolate(),
                                FunctionRouter, local_data));
  object_template_.NewHandle(manager_->isolate())->Set(
      manager_->isolate(), name.c_str(), function_template);

  router_data_.Append(local_data);
}

void ObjectBackedNativeHandler::RouteAccessor(
    const std::string& name,
    const HandlerGetter& getter,
    const HandlerSetter& setter,
    v8::PropertyAttribute attr) {
  v8::HandleScope handle_scope(manager_->isolate());
  v8::Context::Scope context_scope(manager_->GetV8Context());

  v8::Persistent<v8::Object> data(
      manager_->isolate(), v8::Object::New(manager_->isolate()));
  v8::Local<v8::Object> local_data(
      v8::Local<v8::Object>::New(manager_->isolate(), data));
  if (!getter.is_null()) {
    local_data->Set(
        v8::String::NewFromUtf8(manager_->isolate(), kHandlerGetter),
        v8::External::New(manager_->isolate(), new HandlerGetter(getter)));
  }
  if (!setter.is_null()) {
    local_data->Set(
        v8::String::NewFromUtf8(manager_->isolate(), kHandlerSetter),
        v8::External::New(manager_->isolate(), new HandlerSetter(setter)));
  }
  local_data->Set(
      v8::String::NewFromUtf8(manager_->isolate(), kPropertyName),
      v8::String::NewFromUtf8(manager_->isolate(), name.c_str()));
  object_template_.NewHandle(manager_->isolate())->SetAccessor(
      v8::String::NewFromUtf8(manager_->isolate(), name.c_str()),
      GetterRouter, SetterRouter, local_data, v8::DEFAULT, attr);

  router_data_.Append(local_data);
}

ObjectBackedNativeHandler::ObjectBackedNativeHandler(
    ScriptMessageManager* manager) :
    manager_(manager),
    router_data_(manager->isolate()),
    object_template_(manager->isolate(),
                     v8::ObjectTemplate::New(manager->isolate())) {}

ObjectBackedNativeHandler::~ObjectBackedNativeHandler() {
  v8::HandleScope handle_scope(manager_->isolate());
  v8::Context::Scope context_scope(manager_->GetV8Context());

  for (size_t i = 0; i < router_data_.Size(); ++i) {
    v8::Handle<v8::Object> data = router_data_.Get(i);
    v8::Handle<v8::Value> handler_function_value(
        data->Get(v8::String::NewFromUtf8(manager_->isolate(),
                                          kHandlerFunction)));
    v8::Handle<v8::Value> handler_getter_value(
        data->Get(v8::String::NewFromUtf8(manager_->isolate(),
                                          kHandlerGetter)));
    v8::Handle<v8::Value> handler_setter_value(
        data->Get(v8::String::NewFromUtf8(manager_->isolate(),
                                          kHandlerSetter)));
    if (!handler_function_value.IsEmpty()) {
      delete static_cast<HandlerFunction *>(
          handler_function_value.As<v8::External>()->Value());
      data->Delete(v8::String::NewFromUtf8(manager_->isolate(),
                                           kHandlerFunction));
    }
    if (!handler_getter_value.IsEmpty()) {
      delete static_cast<HandlerGetter *>(
          handler_getter_value.As<v8::External>()->Value());
      data->Delete(v8::String::NewFromUtf8(manager_->isolate(),
                                           kHandlerGetter));
    }
    if (!handler_setter_value.IsEmpty()) {
      delete static_cast<HandlerSetter *>(
          handler_setter_value.As<v8::External>()->Value());
      data->Delete(v8::String::NewFromUtf8(manager_->isolate(),
                                           kHandlerSetter));
    }
    data->Delete(v8::String::NewFromUtf8(manager_->isolate(), kPropertyName));
  }
  router_data_.Clear();
}

v8::Handle<v8::Object> ObjectBackedNativeHandler::NewInstance() {
  return object_template_.NewHandle(manager_->isolate())->NewInstance();
}

} // namespace oxide
