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

#include "oxide_qt_user_script_adapter_p.h"

#include <string>

#include "base/bind.h"
#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "content/public/browser/browser_thread.h"

#include "shared/browser/oxide_user_script_master.h"
#include "shared/common/oxide_file_utils.h"

namespace oxide {
namespace qt {

UserScriptAdapterPrivate::UserScriptAdapterPrivate(
    UserScriptAdapter* adapter) :
    state_(UserScriptAdapter::Constructing),
    a(adapter),
    weak_factory_(this) {}

void UserScriptAdapterPrivate::OnGotFileContents(base::PlatformFileError error,
                                                 const char* data,
                                                 int bytes_read) {
  DCHECK_EQ(state_, UserScriptAdapter::Loading);

  if (error != base::PLATFORM_FILE_OK) {
    state_ = UserScriptAdapter::Failed;
    a->OnScriptLoadFailed();
    return;
  }

  std::string str(data, bytes_read);
  user_script_.set_content(str);
  oxide::UserScriptMaster::ParseMetadata(&user_script_);
  state_ = UserScriptAdapter::Ready;

  a->OnScriptLoaded();
}

// static
UserScriptAdapterPrivate* UserScriptAdapterPrivate::Create(
    UserScriptAdapter* adapter) {
  return new UserScriptAdapterPrivate(adapter);
}

void UserScriptAdapterPrivate::StartLoading() {
  DCHECK_EQ(state_, UserScriptAdapter::Constructing);
  state_ = UserScriptAdapter::Loading;

  DCHECK(user_script_.url().scheme() == "file");

  if (!oxide::FileUtils::GetFileContents(
      content::BrowserThread::GetMessageLoopProxyForThread(
        content::BrowserThread::FILE).get(),
      base::FilePath(user_script_.url().path()),
      base::Bind(&UserScriptAdapterPrivate::OnGotFileContents,
                 weak_factory_.GetWeakPtr()))) {
    state_ = UserScriptAdapter::Failed;
    a->OnScriptLoadFailed();
  }
}

// static
UserScriptAdapterPrivate* UserScriptAdapterPrivate::get(
    UserScriptAdapter* adapter) {
  return adapter->priv.data();
}

} // namespace qt
} // namespace oxide
