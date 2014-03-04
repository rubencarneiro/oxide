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

#ifndef _OXIDE_SHARED_RENDERER_V8_SCOPED_PERSISTENT_H_
#define _OXIDE_SHARED_RENDERER_V8_SCOPED_PERSISTENT_H_

#include "v8/include/v8.h"

namespace oxide {

template <typename T>
class ScopedPersistent {
 public:
  ScopedPersistent() {}

  ScopedPersistent(v8::Isolate* isolate, v8::Handle<T> handle) {
    reset(isolate, handle);
  }

  ~ScopedPersistent() {
    reset();
  }

  bool IsEmpty() const {
    return handle_.IsEmpty();
  }

  void reset(v8::Isolate* isolate, v8::Handle<T> handle) {
    if (!handle.IsEmpty()) {
      handle_.Reset(isolate, handle);
    } else {
      reset();
    }
  }

  void reset() {
    handle_.Reset();
  }

  v8::Handle<T> NewHandle(v8::Isolate* isolate) const {
    if (handle_.IsEmpty()) {
      return v8::Local<T>();
    }

    return v8::Local<T>::New(isolate, handle_);
  }

  template <typename P>
  void SetWeak(P* parameters,
               typename v8::WeakCallbackData<T, P>::Callback callback) {
    handle_.SetWeak(parameters, callback);
  }

 private:
  v8::Persistent<T> handle_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_V8_SCOPED_PERSISTENT_H_
