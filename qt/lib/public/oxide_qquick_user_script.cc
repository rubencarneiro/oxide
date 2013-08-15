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

#include "oxide_qquick_user_script.h"

#include <QtDebug>

#include "base/bind.h"
#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
#include "base/platform_file.h"
#include "content/public/browser/browser_thread.h"
#include "url/gurl.h"

#include "shared/browser/oxide_user_script_master.h"
#include "shared/common/oxide_file_utils.h"
#include "shared/common/oxide_user_script.h"

#include "oxide_qquick_user_script_p.h"

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
    constructed_(false),
    state_(OxideQQuickUserScript::Loading),
    user_script_(new oxide::UserScript()),
    weak_factory_(this) {}

OxideQQuickUserScriptPrivate::~OxideQQuickUserScriptPrivate() {}

void OxideQQuickUserScriptPrivate::componentComplete() {
  Q_Q(OxideQQuickUserScript);

  Q_ASSERT(!constructed_);
  constructed_ = true;

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

// static
OxideQQuickUserScriptPrivate* OxideQQuickUserScriptPrivate::get(
    OxideQQuickUserScript* user_script) {
  return user_script->d_func();
}

OxideQQuickUserScript::OxideQQuickUserScript(QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQQuickUserScriptPrivate(this)) {}

OxideQQuickUserScript::~OxideQQuickUserScript() {}

void OxideQQuickUserScript::classBegin() {}

void OxideQQuickUserScript::componentComplete() {
  Q_D(OxideQQuickUserScript);

  d->componentComplete();
}

OxideQQuickUserScript::State OxideQQuickUserScript::state() const {
  Q_D(const OxideQQuickUserScript);

  return d->state();
}

QUrl OxideQQuickUserScript::url() const {
  Q_D(const OxideQQuickUserScript);

  return QUrl(QString::fromStdString(d->user_script()->url().spec()));
}

void OxideQQuickUserScript::setUrl(const QUrl& url) {
  Q_D(OxideQQuickUserScript);

  if (d->constructed()) {
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

bool OxideQQuickUserScript::emulateGreasemonkey() const {
  Q_D(const OxideQQuickUserScript);

  return d->user_script()->emulate_greasemonkey();
}

void OxideQQuickUserScript::setEmulateGreasemonkey(bool emulate_greasemonkey) {
  Q_D(OxideQQuickUserScript);

  if (emulate_greasemonkey == emulateGreasemonkey()) {
    return;
  }

  d->user_script()->set_emulate_greasemonkey(emulate_greasemonkey);
  emit scriptPropertyChanged();
}

bool OxideQQuickUserScript::matchAllFrames() const {
  Q_D(const OxideQQuickUserScript);

  return d->user_script()->match_all_frames();
}

void OxideQQuickUserScript::setMatchAllFrames(bool match_all_frames) {
  Q_D(OxideQQuickUserScript);

  if (match_all_frames == matchAllFrames()) {
    return;
  }

  d->user_script()->set_match_all_frames(match_all_frames);
  emit scriptPropertyChanged();
}

bool OxideQQuickUserScript::incognitoEnabled() const {
  Q_D(const OxideQQuickUserScript);

  return d->user_script()->incognito_enabled();
}

void OxideQQuickUserScript::setIncognitoEnabled(bool incognito_enabled) {
  Q_D(OxideQQuickUserScript);

  if (incognito_enabled == incognitoEnabled()) {
    return;
  }

  d->user_script()->set_incognito_enabled(incognito_enabled);
  emit scriptPropertyChanged();
}

QString OxideQQuickUserScript::worldId() const {
  Q_D(const OxideQQuickUserScript);

  return QString::fromStdString(d->user_script()->world_id());
}

void OxideQQuickUserScript::setWorldId(const QString& world_id) {
  Q_D(OxideQQuickUserScript);

  if (world_id == worldId()) {
    return;
  }

  d->user_script()->set_world_id(world_id.toStdString());
  emit scriptPropertyChanged();
}
