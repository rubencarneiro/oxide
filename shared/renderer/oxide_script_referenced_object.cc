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

#include "oxide_script_referenced_object.h"

#include "base/logging.h"

#include "oxide_script_message_manager.h"

namespace oxide {

namespace {
const char kWrappedNativeObject[] = "__oxide_wrapped_native_object";
}

ScriptReferencedObjectBase::ScriptReferencedObjectBase(
    ScriptMessageManager* mm,
    v8::Handle<v8::Object> handle) :
    manager_(mm->AsWeakPtr()),
    handle_(manager_->isolate(), handle) {
  v8::Isolate* isolate = manager_->isolate();
  v8::HandleScope handle_scope(isolate);
  v8::Context::Scope context_scope(manager_->GetV8Context());

  data_.reset(isolate, v8::External::New(isolate, this));

  if (!handle.IsEmpty()) {
    handle->SetHiddenValue(
        v8::String::NewFromUtf8(isolate, kWrappedNativeObject),
        data_.NewHandle(isolate));
  }
}

ScriptReferencedObjectBase::~ScriptReferencedObjectBase() {
  CHECK(handle_.IsEmpty());
}

v8::Handle<v8::Object> ScriptReferencedObjectBase::GetHandle() const {
  CHECK(manager_);
  return handle_.NewHandle(manager_->isolate());
}

// static
ScriptReferencedObjectBase* ScriptReferencedObjectBase::FromScriptHandle(
    const v8::Handle<v8::Object>& handle) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  v8::Local<v8::Value> val(handle->GetHiddenValue(
      v8::String::NewFromUtf8(isolate, kWrappedNativeObject)));
  if (val.IsEmpty() || !val->IsExternal()) {
    return NULL;
  }

  return reinterpret_cast<ScriptReferencedObjectBase *>(
      val.As<v8::External>()->Value());
}

} // namespace oxide
