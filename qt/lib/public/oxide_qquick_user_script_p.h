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

#ifndef _OXIDE_QT_LIB_PUBLIC_QQUICK_USER_SCRIPT_P_H_
#define _OXIDE_QT_LIB_PUBLIC_QQUICK_USER_SCRIPT_P_H_

#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/platform_file.h"

#include "oxide_qquick_user_script.h"

namespace oxide {
class UserScript;
}

class OxideQQuickUserScriptPrivate {
  Q_DECLARE_PUBLIC(OxideQQuickUserScript)

 public:
  OxideQQuickUserScriptPrivate(OxideQQuickUserScript* q);
  virtual ~OxideQQuickUserScriptPrivate();

  void componentComplete();

  bool constructed() const { return constructed_; }
  OxideQQuickUserScript::State state() const { return state_; }
  oxide::UserScript* user_script() const {
    return user_script_.get();
  }

  static OxideQQuickUserScriptPrivate* get(OxideQQuickUserScript* user_script);

 private:
  void OnGotFileContents(base::PlatformFileError error,
                         const char* data,
                         int bytes_read);

  OxideQQuickUserScript* q_ptr;
  bool constructed_;
  OxideQQuickUserScript::State state_;
  scoped_ptr<oxide::UserScript> user_script_;
  base::WeakPtrFactory<OxideQQuickUserScriptPrivate> weak_factory_;
};

#endif // _OXIDE_QT_LIB_PUBLIC_QQUICK_USER_SCRIPT_P_H_
