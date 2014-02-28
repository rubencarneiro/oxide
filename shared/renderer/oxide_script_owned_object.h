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

#ifndef _OXIDE_SHARED_RENDERER_SCRIPT_OWNED_OBJECT_H_
#define _OXIDE_SHARED_RENDERER_SCRIPT_OWNED_OBJECT_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/weak_ptr.h"
#include "v8/include/v8.h"

#include "shared/renderer/oxide_v8_scoped_persistent.h"

namespace oxide {

class ScriptMessageManager;

class ScriptOwnedObject {
 public:
  ScriptOwnedObject(ScriptMessageManager* mm,
                    const v8::Handle<v8::Object>& handle);
  virtual ~ScriptOwnedObject();

  v8::Handle<v8::Object> GetHandle() const;

  static ScriptOwnedObject* FromScriptHandle(
      const v8::Handle<v8::Object>& handle);

 protected:
  ScriptMessageManager* manager() const { return manager_.get(); }

 private:
  static void NearDeathCallback(
      const v8::WeakCallbackData<v8::Object, ScriptOwnedObject>& data);

  base::WeakPtr<ScriptMessageManager> manager_;
  ScopedPersistent<v8::Object> handle_;
  ScopedPersistent<v8::External> data_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(ScriptOwnedObject);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_SCRIPT_OWNED_OBJECT_H_
