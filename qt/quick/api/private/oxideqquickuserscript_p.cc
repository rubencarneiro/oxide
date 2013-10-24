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

#include "qt/quick/api/oxideqquickuserscript_p_p.h"

#include <QString>

#include "base/bind.h"
#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/platform_file.h"
#include "content/public/browser/browser_thread.h"
#include "url/gurl.h"

#include "shared/browser/oxide_user_script_master.h"
#include "shared/common/oxide_file_utils.h"
#include "shared/common/oxide_user_script.h"

void OxideQQuickUserScriptPrivate::OnGotFileContents(
    base::PlatformFileError error,
    const char* data,
    int bytes_read) {
  Q_Q(OxideQQuickUserScript);

  Q_ASSERT(state_ == OxideQQuickUserScript::Loading);

  if (error != base::PLATFORM_FILE_OK) {
    state_ = OxideQQuickUserScript::Failed;
    emit q->scriptLoadFailed();
    return;
  }

  std::string str(data, bytes_read);
  user_script_->set_content(str);
  oxide::UserScriptMaster::ParseMetadata(user_script_.get());
  state_ = OxideQQuickUserScript::Ready;

  emit q->scriptLoaded();  
}

OxideQQuickUserScriptPrivate::OxideQQuickUserScriptPrivate(
    OxideQQuickUserScript* q) :
    q_ptr(q),
    state_(OxideQQuickUserScript::Constructing),
    user_script_(new oxide::UserScript()),
    weak_factory_(this) {}

void OxideQQuickUserScriptPrivate::startLoading() {
  Q_Q(OxideQQuickUserScript);

  Q_ASSERT(state_ == OxideQQuickUserScript::Constructing);
  state_ = OxideQQuickUserScript::Loading;

  Q_ASSERT(user_script_->url().scheme() == "file");

  if (!oxide::FileUtils::GetFileContents(
      content::BrowserThread::GetMessageLoopProxyForThread(
        content::BrowserThread::FILE).get(),
      base::FilePath(user_script_->url().path()),
      base::Bind(&OxideQQuickUserScriptPrivate::OnGotFileContents,
                 weak_factory_.GetWeakPtr()))) {
    state_ = OxideQQuickUserScript::Failed;
    emit q->scriptLoadFailed();
  }
}

QUrl OxideQQuickUserScriptPrivate::url() const {
  return QUrl(QString::fromStdString(user_script_->url().spec()));
}

void OxideQQuickUserScriptPrivate::setUrl(const QUrl& url) {
  user_script_->set_url(GURL(url.toString().toStdString()));
}

// static
OxideQQuickUserScriptPrivate* OxideQQuickUserScriptPrivate::get(
    OxideQQuickUserScript* user_script) {
  return user_script->d_func();
}
