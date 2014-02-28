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

#include "oxide_script_owned_object.h"

#include "base/logging.h"

#include "oxide_script_message_manager.h"

namespace oxide {

namespace {
const char kWrappedNativeObject[] = "__oxide_wrapped_native_object";
}

// static
void ScriptOwnedObject::NearDeathCallback(
    const v8::WeakCallbackData<v8::Object, ScriptOwnedObject>& data) {
  ScriptOwnedObject* self = data.GetParameter();
  self->handle_.reset();
  delete self;
}

ScriptOwnedObject::ScriptOwnedObject(ScriptMessageManager* mm,
                                     const v8::Handle<v8::Object>& handle) :
    manager_(mm->AsWeakPtr()),
    handle_(manager_->isolate(), handle) {
  v8::Isolate* isolate = manager_->isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Context::Scope context_scope(manager_->GetV8Context());

  data_.reset(isolate, v8::External::New(isolate, this));

  handle->SetHiddenValue(
      v8::String::NewFromUtf8(isolate, kWrappedNativeObject),
      data_.NewHandle(isolate));
  handle_.SetWeak(this, NearDeathCallback);
}

ScriptOwnedObject::~ScriptOwnedObject() {
  if (!handle_.IsEmpty()) {
    CHECK(manager_);
    v8::Isolate* isolate = manager_->isolate();
    v8::HandleScope handle_scope(isolate);
    v8::Context::Scope context_scope(manager_->GetV8Context());
    v8::Handle<v8::Object> handle(handle_.NewHandle(isolate));
    handle->DeleteHiddenValue(
        v8::String::NewFromUtf8(isolate, kWrappedNativeObject));
  }
}

v8::Handle<v8::Object> ScriptOwnedObject::GetHandle() const {
  CHECK(manager_);
  return handle_.NewHandle(manager_->isolate());
}

// static
ScriptOwnedObject* ScriptOwnedObject::FromScriptHandle(
    const v8::Handle<v8::Object>& handle) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  v8::Local<v8::Value> val(handle->GetHiddenValue(
      v8::String::NewFromUtf8(isolate, kWrappedNativeObject)));
  if (val.IsEmpty() || !val->IsExternal()) {
    return NULL;
  }

  return reinterpret_cast<ScriptOwnedObject *>(val.As<v8::External>()->Value());
}

} // namespace oxide
