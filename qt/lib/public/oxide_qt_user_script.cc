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

#include <QtDebug>

#include "base/bind.h"
#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/platform_file.h"
#include "content/public/browser/browser_thread.h"
#include "url/gurl.h"

#include "shared/browser/oxide_user_script_master.h"
#include "shared/common/oxide_file_utils.h"
#include "shared/common/oxide_user_script.h"

#include "oxide_qt_user_script_p.h"

namespace oxide {
namespace qt {

void UserScriptPrivate::OnGotFileContents(
    base::PlatformFileError error,
    const char* data,
    int bytes_read) {
  Q_Q(UserScript);

  Q_ASSERT(state_ == UserScript::Loading);

  if (error != base::PLATFORM_FILE_OK) {
    state_ = UserScript::Failed;
    emit q->scriptLoadFailed();
    return;
  }

  std::string str(data, bytes_read);
  user_script_->set_content(str);
  oxide::UserScriptMaster::ParseMetadata(user_script_.get());
  state_ = UserScript::Ready;

  emit q->scriptLoaded();  
}

UserScriptPrivate::UserScriptPrivate(UserScript* q) :
    q_ptr(q),
    state_(UserScript::Constructing),
    user_script_(new oxide::UserScript()),
    weak_factory_(this) {}

UserScriptPrivate::~UserScriptPrivate() {}

void UserScriptPrivate::startLoading() {
  Q_Q(UserScript);

  Q_ASSERT(state_ == UserScript::Constructing);
  state_ = UserScript::Loading;

  Q_ASSERT(user_script_->url().scheme() == "file");

  if (!oxide::FileUtils::GetFileContents(
      content::BrowserThread::GetMessageLoopProxyForThread(
        content::BrowserThread::FILE).get(),
      base::FilePath(user_script_->url().path()),
      base::Bind(&UserScriptPrivate::OnGotFileContents,
                 weak_factory_.GetWeakPtr()))) {
    state_ = UserScript::Failed;
    emit q->scriptLoadFailed();
  }
}

// static
UserScriptPrivate* UserScriptPrivate::get(UserScript* user_script) {
  return user_script->d_func();
}

UserScript::UserScript(UserScriptPrivate& dd, QObject* parent) :
    QObject(parent),
    d_ptr(&dd) {}

UserScript::~UserScript() {}

void UserScript::startLoading() {
  Q_D(UserScript);

  d->startLoading();
}

UserScript::State UserScript::state() const {
  Q_D(const UserScript);

  return d->state();
}

QUrl UserScript::url() const {
  Q_D(const UserScript);

  return QUrl(QString::fromStdString(d->user_script()->url().spec()));
}

void UserScript::setUrl(const QUrl& url) {
  Q_D(UserScript);

  if (d->state() != Constructing) {
    qWarning() << "url is a construct-only parameter";
    return;
  }

  if (!url.isLocalFile()) {
    qWarning() << "Only local files are currently supported";
    return;
  }

  if (!url.isValid()) {
    qWarning() << "Invalid URL";
    return;
  }

  d->user_script()->set_url(GURL(url.toString().toStdString()));
}

bool UserScript::emulateGreasemonkey() const {
  Q_D(const UserScript);

  return d->user_script()->emulate_greasemonkey();
}

void UserScript::setEmulateGreasemonkey(bool emulate_greasemonkey) {
  Q_D(UserScript);

  if (emulate_greasemonkey == emulateGreasemonkey()) {
    return;
  }

  d->user_script()->set_emulate_greasemonkey(emulate_greasemonkey);
  emit scriptPropertyChanged();
}

bool UserScript::matchAllFrames() const {
  Q_D(const UserScript);

  return d->user_script()->match_all_frames();
}

void UserScript::setMatchAllFrames(bool match_all_frames) {
  Q_D(UserScript);

  if (match_all_frames == matchAllFrames()) {
    return;
  }

  d->user_script()->set_match_all_frames(match_all_frames);
  emit scriptPropertyChanged();
}

bool UserScript::incognitoEnabled() const {
  Q_D(const UserScript);

  return d->user_script()->incognito_enabled();
}

void UserScript::setIncognitoEnabled(bool incognito_enabled) {
  Q_D(UserScript);

  if (incognito_enabled == incognitoEnabled()) {
    return;
  }

  d->user_script()->set_incognito_enabled(incognito_enabled);
  emit scriptPropertyChanged();
}

QString UserScript::worldId() const {
  Q_D(const UserScript);

  return QString::fromStdString(d->user_script()->world_id());
}

void UserScript::setWorldId(const QString& world_id) {
  Q_D(UserScript);

  if (world_id == worldId()) {
    return;
  }

  d->user_script()->set_world_id(world_id.toStdString());
  emit scriptPropertyChanged();
}

} // namespace qt
} // namespace oxide
