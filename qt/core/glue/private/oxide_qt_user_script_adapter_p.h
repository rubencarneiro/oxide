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

#ifndef _OXIDE_QT_CORE_GLUE_PRIVATE_USER_SCRIPT_ADAPTER_H_
#define _OXIDE_QT_CORE_GLUE_PRIVATE_USER_SCRIPT_ADAPTER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/weak_ptr.h"
#include "base/platform_file.h"

#include "shared/common/oxide_user_script.h"

#include "qt/core/glue/oxide_qt_user_script_adapter.h"

namespace oxide {
namespace qt {

class UserScriptAdapter;

class UserScriptAdapterPrivate FINAL {
 public:
  static UserScriptAdapterPrivate* Create(UserScriptAdapter* adapter);

  const oxide::UserScript& user_script() const {
    return user_script_;
  }
  oxide::UserScript& user_script() {
    return user_script_;
  }

  UserScriptAdapter::State state() const {
    return state_;
  }

  void StartLoading();

  static UserScriptAdapterPrivate* get(UserScriptAdapter* adapter);

 private:
  friend class UserScriptAdapter;

  UserScriptAdapterPrivate(UserScriptAdapter* adapter);
  void OnGotFileContents(base::PlatformFileError error,
                         const char* data,
                         int bytes_read);

  UserScriptAdapter::State state_;
  oxide::UserScript user_script_;
  UserScriptAdapter* a;
  base::WeakPtrFactory<UserScriptAdapterPrivate> weak_factory_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(UserScriptAdapterPrivate);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_PRIVATE_USER_SCRIPT_ADAPTER_H_
