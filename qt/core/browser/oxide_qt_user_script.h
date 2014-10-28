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

#ifndef _OXIDE_QT_CORE_BROWSER_USER_SCRIPT_H_
#define _OXIDE_QT_CORE_BROWSER_USER_SCRIPT_H_

#include <string>

#include "base/basictypes.h"
#include "base/macros.h"
#include "base/files/file.h"
#include "url/gurl.h"

#include "shared/common/oxide_user_script.h"

namespace oxide {

class AsyncFileJob;

namespace qt {

class UserScriptAdapter;

class UserScript final {
 public:
  enum State {
    Constructing,
    Loading,
    Loaded,
    FailedLoad
  };

  ~UserScript();

  static UserScript* FromAdapter(UserScriptAdapter* adapter);

  const oxide::UserScript* impl() const { return &impl_; }
  State state() const { return state_; }

  bool GetEmulateGreasemonkey() const;
  void SetEmulateGreasemonkey(bool emulate);

  bool GetMatchAllFrames() const;
  void SetMatchAllFrames(bool match);

  bool GetIncognitoEnabled() const;
  void SetIncognitoEnabled(bool enabled);

  GURL GetContext() const;
  void SetContext(const GURL& context);

 private:
  friend class UserScriptAdapter;

  UserScript(UserScriptAdapter* adapter);

  void Init(const base::FilePath& path);
  void OnGotFileContents(base::File::Error error,
                         const char* data,
                         int bytes_read);

  UserScriptAdapter* adapter_;

  State state_;

  oxide::UserScript impl_;

  scoped_ptr<AsyncFileJob> load_job_;

  DISALLOW_COPY_AND_ASSIGN(UserScript);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_USER_SCRIPT_H_
