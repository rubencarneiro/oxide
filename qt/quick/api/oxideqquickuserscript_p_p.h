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

#ifndef _OXIDE_QT_QUICK_API_USER_SCRIPT_P_P_H_
#define _OXIDE_QT_QUICK_API_USER_SCRIPT_P_P_H_

#include <QtGlobal>
#include <QUrl>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/platform_file.h"

#include "qt/quick/api/oxideqquickuserscript_p.h"

namespace oxide {
class UserScript;
}

class OxideQQuickUserScriptPrivate FINAL {
  Q_DECLARE_PUBLIC(OxideQQuickUserScript)

 public:
  OxideQQuickUserScriptPrivate(OxideQQuickUserScript* q);

  void startLoading();

  OxideQQuickUserScript::State state() const { return state_; }
  oxide::UserScript* user_script() const {
    return user_script_.get();
  }

  QUrl url() const;
  void setUrl(const QUrl& url);
 
  static OxideQQuickUserScriptPrivate* get(OxideQQuickUserScript* user_script);

 protected:
  OxideQQuickUserScript* q_ptr;

 private:
  void OnGotFileContents(base::PlatformFileError error,
                         const char* data,
                         int bytes_read);

  OxideQQuickUserScript::State state_;
  scoped_ptr<oxide::UserScript> user_script_;
  base::WeakPtrFactory<OxideQQuickUserScriptPrivate> weak_factory_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(OxideQQuickUserScriptPrivate);
};

#endif // _OXIDE_QT_QUICK_API_USER_SCRIPT_P_P_H_
