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

#include "oxide_qt_user_script.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "content/public/browser/browser_thread.h"

#include "qt/core/glue/oxide_qt_user_script_adapter.h"
#include "shared/browser/oxide_browser_process_main.h"
#include "shared/browser/oxide_user_script_master.h"
#include "shared/common/oxide_file_utils.h"

namespace oxide {
namespace qt {

UserScript::UserScript(UserScriptAdapter* adapter)
    : adapter_(adapter),
      state_(Constructing) {}

void UserScript::Init(const base::FilePath& path) {
  DCHECK_EQ(state_, Constructing);

  state_ = Loading;

  load_job_.reset(oxide::FileUtils::GetFileContents(
      content::BrowserThread::GetMessageLoopProxyForThread(
        content::BrowserThread::FILE).get(),
      path,
      base::Bind(&UserScript::OnGotFileContents, base::Unretained(this))));
  if (!load_job_) {
    state_ = FailedLoad;
    adapter_->OnScriptLoadFailed();
  }
}

void UserScript::OnGotFileContents(base::File::Error error,
                                   const char* data,
                                   int bytes_read) {
  DCHECK_EQ(state_, Loading);

  if (error != base::File::FILE_OK) {
    state_ = FailedLoad;
    adapter_->OnScriptLoadFailed();
    return;
  }

  std::string str(data, bytes_read);
  impl_.set_content(str);
  oxide::UserScriptMaster::ParseMetadata(&impl_);
  state_ = Loaded;

  adapter_->OnScriptLoaded();
}

UserScript::~UserScript() {}

// static
UserScript* UserScript::FromAdapter(UserScriptAdapter* adapter) {
  return adapter->script_.data();
}

bool UserScript::GetEmulateGreasemonkey() const {
  return impl_.emulate_greasemonkey();
}

void UserScript::SetEmulateGreasemonkey(bool emulate) {
  impl_.set_emulate_greasemonkey(emulate);
}

bool UserScript::GetMatchAllFrames() const {
  return impl_.match_all_frames();
}

void UserScript::SetMatchAllFrames(bool match) {
  impl_.set_match_all_frames(match);
}

bool UserScript::GetIncognitoEnabled() const {
  return impl_.incognito_enabled();
}

void UserScript::SetIncognitoEnabled(bool enabled) {
  impl_.set_incognito_enabled(enabled);
}

GURL UserScript::GetContext() const {
  return impl_.context();
}

void UserScript::SetContext(const GURL& context) {
  impl_.set_context(context);
}

} // namespace qt
} // namespace oxide
