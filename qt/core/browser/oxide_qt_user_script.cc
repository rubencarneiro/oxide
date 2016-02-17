// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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
#include "base/files/file_path.h"
#include "base/logging.h"
#include "content/public/browser/browser_thread.h"
#include "url/gurl.h"

#include "qt/core/glue/oxide_qt_user_script_proxy_client.h"
#include "shared/browser/oxide_browser_process_main.h"
#include "shared/browser/oxide_user_script_master.h"
#include "shared/common/oxide_file_utils.h"

namespace oxide {
namespace qt {

void UserScript::OnGotFileContents(base::File::Error error,
                                   const char* data,
                                   int bytes_read) {
  DCHECK_EQ(state_, Loading);

  if (error != base::File::FILE_OK) {
    state_ = FailedLoad;
    client_->ScriptLoadFailed();
    return;
  }

  std::string str(data, bytes_read);
  impl_.set_content(str);
  oxide::UserScriptMaster::ParseMetadata(&impl_);
  state_ = Loaded;

  client_->ScriptLoaded();
}

bool UserScript::emulateGreasemonkey() const {
  return impl_.emulate_greasemonkey();
}

void UserScript::setEmulateGreasemonkey(bool emulate) {
  impl_.set_emulate_greasemonkey(emulate);
}

bool UserScript::matchAllFrames() const {
  return impl_.match_all_frames();
}

void UserScript::setMatchAllFrames(bool match) {
  impl_.set_match_all_frames(match);
}

bool UserScript::incognitoEnabled() const {
  return impl_.incognito_enabled();
}

void UserScript::setIncognitoEnabled(bool enabled) {
  impl_.set_incognito_enabled(enabled);
}

QUrl UserScript::context() const {
  return QUrl(QString::fromStdString(impl_.context().spec()));
}

void UserScript::setContext(const QUrl& context) {
  impl_.set_context(GURL(context.toString().toStdString()));
}

UserScript::UserScript(UserScriptProxyClient* client,
                       QObject* handle,
                       const QUrl& url)
    : client_(client),
      state_(Loading) {
  DCHECK(client);
  DCHECK(handle);

  setHandle(handle);

  load_job_.reset(oxide::FileUtils::GetFileContents(
      content::BrowserThread::GetMessageLoopProxyForThread(
        content::BrowserThread::FILE).get(),
      base::FilePath(url.toLocalFile().toStdString()),
      base::Bind(&UserScript::OnGotFileContents,
                 // The callback won't run after |this| is deleted because
                 // it cancels the job
                 base::Unretained(this))));
  if (!load_job_) {
    state_ = FailedLoad;
    client_->ScriptLoadFailed();
  }
}

UserScript::~UserScript() {}

} // namespace qt
} // namespace oxide
