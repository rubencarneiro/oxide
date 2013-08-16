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

#ifndef _OXIDE_QT_LIB_PUBLIC_USER_SCRIPT_P_H_
#define _OXIDE_QT_LIB_PUBLIC_USER_SCRIPT_P_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/platform_file.h"

#include "oxide_qt_user_script.h"

namespace oxide {

class UserScript;

namespace qt {

class UserScriptPrivate FINAL {
  Q_DECLARE_PUBLIC(UserScript)

 public:
  UserScriptPrivate(UserScript* q);
  virtual ~UserScriptPrivate();

  void startLoading();

  UserScript::State state() const { return state_; }
  oxide::UserScript* user_script() const {
    return user_script_.get();
  }
 
  static UserScriptPrivate* get(UserScript* user_script);

 protected:
  UserScript* q_ptr;

 private:
  void OnGotFileContents(base::PlatformFileError error,
                         const char* data,
                         int bytes_read);

  UserScript::State state_;
  scoped_ptr<oxide::UserScript> user_script_;
  base::WeakPtrFactory<UserScriptPrivate> weak_factory_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(UserScriptPrivate);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_LIB_PUBLIC_USER_SCRIPT_P_H
