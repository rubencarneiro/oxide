// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.
// Copyright (C) 2013 The Chromium Authors

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

#ifndef _OXIDE_SHARED_RENDERER_V8_UNSAFE_PERSISTENT_H_
#define _OXIDE_SHARED_RENDERER_V8_UNSAFE_PERSISTENT_H_

#include "v8/include/v8.h"

namespace oxide {

template <typename T>
class UnsafePersistent {
 public:
  UnsafePersistent() : value_(0) {}

  explicit UnsafePersistent(v8::Persistent<T>* handle) {
    value_ = handle->ClearAndLeak();
  }

  UnsafePersistent(v8::Isolate* isolate, const v8::Handle<T>& handle) {
    v8::Persistent<T> persistent(isolate, handle);
    value_ = persistent.ClearAndLeak();
  }

  void dispose() {
    v8::Persistent<T> handle(value_);
    handle.Reset();
    value_ = 0;
  }

  v8::Local<T> newLocal(v8::Isolate* isolate) {
    return v8::Local<T>::New(isolate, v8::Local<T>(value_));
  }

 private:
  T* value_;
};
 
} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_V8_UNSAFE_PERSISTENT_H_
