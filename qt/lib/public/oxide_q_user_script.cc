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

#include "oxide_q_user_script.h"

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

#include "oxide_q_user_script_p.h"

void OxideQUserScriptPrivate::OnGotFileContents(
    base::PlatformFileError error,
    const char* data,
    int bytes_read) {
  Q_Q(OxideQUserScript);

  Q_ASSERT(state_ == OxideQUserScript::Loading);

  if (error != base::PLATFORM_FILE_OK) {
    state_ = OxideQUserScript::Failed;
    emit q->scriptLoadFailed();
    return;
  }

  std::string str(data, bytes_read);
  user_script_->set_content(str);
  oxide::UserScriptMaster::ParseMetadata(user_script_.get());
  state_ = OxideQUserScript::Ready;

  emit q->scriptLoaded();  
}

OxideQUserScriptPrivate::OxideQUserScriptPrivate(OxideQUserScript* q) :
    q_ptr(q),
    state_(OxideQUserScript::Constructing),
    user_script_(new oxide::UserScript()),
    weak_factory_(this) {}

OxideQUserScriptPrivate::~OxideQUserScriptPrivate() {}

void OxideQUserScriptPrivate::startLoading() {
  Q_Q(OxideQUserScript);

  Q_ASSERT(state_ == OxideQUserScript::Constructing);
  state_ = OxideQUserScript::Loading;

  Q_ASSERT(user_script_->url().scheme() == "file");

  if (!oxide::FileUtils::GetFileContents(
      content::BrowserThread::GetMessageLoopProxyForThread(
        content::BrowserThread::FILE).get(),
      base::FilePath(user_script_->url().path()),
      base::Bind(&OxideQUserScriptPrivate::OnGotFileContents,
                 weak_factory_.GetWeakPtr()))) {
    state_ = OxideQUserScript::Failed;
    emit q->scriptLoadFailed();
  }
}

// static
OxideQUserScriptPrivate* OxideQUserScriptPrivate::get(
    OxideQUserScript* user_script) {
  return user_script->d_func();
}

OxideQUserScript::OxideQUserScript(QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQUserScriptPrivate(this)) {}

OxideQUserScript::~OxideQUserScript() {}

void OxideQUserScript::classBegin() {}

void OxideQUserScript::componentComplete() {
  startLoading();
}

void OxideQUserScript::startLoading() {
  Q_D(OxideQUserScript);

  d->startLoading();
}

OxideQUserScript::State OxideQUserScript::state() const {
  Q_D(const OxideQUserScript);

  return d->state();
}

QUrl OxideQUserScript::url() const {
  Q_D(const OxideQUserScript);

  return QUrl(QString::fromStdString(d->user_script()->url().spec()));
}

void OxideQUserScript::setUrl(const QUrl& url) {
  Q_D(OxideQUserScript);

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

bool OxideQUserScript::emulateGreasemonkey() const {
  Q_D(const OxideQUserScript);

  return d->user_script()->emulate_greasemonkey();
}

void OxideQUserScript::setEmulateGreasemonkey(bool emulate_greasemonkey) {
  Q_D(OxideQUserScript);

  if (emulate_greasemonkey == emulateGreasemonkey()) {
    return;
  }

  d->user_script()->set_emulate_greasemonkey(emulate_greasemonkey);
  emit scriptPropertyChanged();
}

bool OxideQUserScript::matchAllFrames() const {
  Q_D(const OxideQUserScript);

  return d->user_script()->match_all_frames();
}

void OxideQUserScript::setMatchAllFrames(bool match_all_frames) {
  Q_D(OxideQUserScript);

  if (match_all_frames == matchAllFrames()) {
    return;
  }

  d->user_script()->set_match_all_frames(match_all_frames);
  emit scriptPropertyChanged();
}

bool OxideQUserScript::incognitoEnabled() const {
  Q_D(const OxideQUserScript);

  return d->user_script()->incognito_enabled();
}

void OxideQUserScript::setIncognitoEnabled(bool incognito_enabled) {
  Q_D(OxideQUserScript);

  if (incognito_enabled == incognitoEnabled()) {
    return;
  }

  d->user_script()->set_incognito_enabled(incognito_enabled);
  emit scriptPropertyChanged();
}

QString OxideQUserScript::worldId() const {
  Q_D(const OxideQUserScript);

  return QString::fromStdString(d->user_script()->world_id());
}

void OxideQUserScript::setWorldId(const QString& world_id) {
  Q_D(OxideQUserScript);

  if (world_id == worldId()) {
    return;
  }

  d->user_script()->set_world_id(world_id.toStdString());
  emit scriptPropertyChanged();
}
