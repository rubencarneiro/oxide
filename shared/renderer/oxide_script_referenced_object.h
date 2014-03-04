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

#ifndef _OXIDE_SHARED_RENDERER_SCRIPT_REFERENCED_OBJECT_H_
#define _OXIDE_SHARED_RENDERER_SCRIPT_REFERENCED_OBJECT_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "v8/include/v8.h"

#include "shared/renderer/oxide_v8_scoped_persistent.h"

namespace oxide {

class ScriptMessageManager;

class ScriptReferencedObjectBase {
 public:
  ScriptReferencedObjectBase(ScriptMessageManager* mm,
                             v8::Handle<v8::Object> handle);
  virtual ~ScriptReferencedObjectBase();

  v8::Handle<v8::Object> GetHandle() const;

 protected:
  ScriptMessageManager* manager() const { return manager_.get(); }

  static ScriptReferencedObjectBase* FromScriptHandle(
      const v8::Handle<v8::Object>& handle);

  template <class U>
  static void Finish(U* self) {
    if (!self->handle_.IsEmpty()) {
      self->handle_.SetWeak(new scoped_refptr<U>(self), NearDeathCallback);
    }
  }

 private:
  template <class U>
  static void NearDeathCallback(
      const v8::WeakCallbackData<v8::Object, scoped_refptr<U> >& data) {
    scoped_refptr<U>* refptr = data.GetParameter();
    (*refptr)->handle_.reset();
    delete refptr;
  }

  base::WeakPtr<ScriptMessageManager> manager_;
  ScopedPersistent<v8::External> data_;
  ScopedPersistent<v8::Object> handle_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(ScriptReferencedObjectBase);
};

// Base class for objects that are referenced by a script handle
// This holds a strong reference to itself when constructed, and releases
// that reference when the corresponding script handle is collected by
// V8.
// Derived classes must already be reference counted - this class does not
// implement reference counting itself
template <class T>
class ScriptReferencedObject : public ScriptReferencedObjectBase {
 public:
  ScriptReferencedObject(ScriptMessageManager* mm,
                         const v8::Handle<v8::Object>& handle) :
      ScriptReferencedObjectBase(mm, handle) {
    ScriptReferencedObjectBase::Finish(static_cast<T *>(this));
  }

  virtual ~ScriptReferencedObject() {}

  static T* FromScriptHandle(const v8::Handle<v8::Object>& handle) {
    return static_cast<T *>(
        ScriptReferencedObjectBase::FromScriptHandle(handle));
  }

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(ScriptReferencedObject);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_SCRIPT_REFERENCED_OBJECT_H_
