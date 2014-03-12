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

#include "oxide_qt_user_script_adapter.h"
#include "oxide_qt_user_script_adapter_p.h"

#include <QDebug>
#include <string>

#include "base/bind.h"
#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "content/public/browser/browser_thread.h"
#include "url/gurl.h"

#include "shared/browser/oxide_browser_process_main.h"
#include "shared/browser/oxide_user_script_master.h"
#include "shared/common/oxide_file_utils.h"
#include "shared/common/oxide_user_script.h"

namespace oxide {
namespace qt {

UserScriptAdapterPrivate::UserScriptAdapterPrivate(
    UserScriptAdapter* adapter) :
    state(Constructing),
    a(adapter) {}

bool UserScriptAdapterPrivate::Load() {
  load_job_.reset(oxide::FileUtils::GetFileContents(
      content::BrowserThread::GetMessageLoopProxyForThread(
        content::BrowserThread::FILE).get(),
      base::FilePath(user_script.url().path()),
      base::Bind(&UserScriptAdapterPrivate::OnGotFileContents,
                 base::Unretained(this))));
  return load_job_ ? true : false;
}

void UserScriptAdapterPrivate::OnGotFileContents(base::File::Error error,
                                                 const char* data,
                                                 int bytes_read) {
  DCHECK_EQ(state, Loading);

  if (error != base::File::FILE_OK) {
    state = FailedLoad;
    a->OnScriptLoadFailed();
    return;
  }

  std::string str(data, bytes_read);
  user_script.set_content(str);
  oxide::UserScriptMaster::ParseMetadata(&user_script);
  state = Loaded;

  a->OnScriptLoaded();
}

// static
UserScriptAdapterPrivate* UserScriptAdapterPrivate::get(
    UserScriptAdapter* adapter) {
  return adapter->priv.data();
}

UserScriptAdapter::UserScriptAdapter(QObject* q) :
    AdapterBase(q),
    priv(new UserScriptAdapterPrivate(this)) {}

UserScriptAdapter::~UserScriptAdapter() {}

QUrl UserScriptAdapter::url() const {
  return QUrl(QString::fromStdString(priv->user_script.url().spec()));
}

void UserScriptAdapter::setUrl(const QUrl& url) {
  if (priv->state != UserScriptAdapterPrivate::Constructing) {
    LOG(WARNING) << "UserScript url is a construct-only parameter";
    return;
  }

  if (!url.isLocalFile()) {
    LOG(WARNING) << "UserScript url must be set to a local file";
    return;
  }

  if (!url.isValid()) {
    LOG(WARNING) << "UserScript url must be set to a valid URL";
    return;
  }

  priv->user_script.set_url(GURL(url.toString().toStdString()));
}

bool UserScriptAdapter::emulateGreasemonkey() const {
  return priv->user_script.emulate_greasemonkey();
}

void UserScriptAdapter::setEmulateGreasemonkey(bool emulate) {
  priv->user_script.set_emulate_greasemonkey(emulate);
}

bool UserScriptAdapter::privateInjectedInMainWorld() const {
  return priv->user_script.inject_in_main_world();
}

void UserScriptAdapter::setPrivateInjectedInMainWorld(bool in_main_world) {
  priv->user_script.set_inject_in_main_world(in_main_world);
}

bool UserScriptAdapter::matchAllFrames() const {
  return priv->user_script.match_all_frames();
}

void UserScriptAdapter::setMatchAllFrames(bool match) {
  priv->user_script.set_match_all_frames(match);
}

bool UserScriptAdapter::incognitoEnabled() const {
  return priv->user_script.incognito_enabled();
}

void UserScriptAdapter::setIncognitoEnabled(bool enabled) {
  priv->user_script.set_incognito_enabled(enabled);
}

QUrl UserScriptAdapter::context() const {
  return QUrl(QString::fromStdString(priv->user_script.context().spec()));
}

void UserScriptAdapter::setContext(const QUrl& context) {
  if (!context.isValid()) {
    LOG(WARNING) << "UserScript context must be set to a valid URL";
    return;
  }

  priv->user_script.set_context(GURL(context.toString().toStdString()));
}

void UserScriptAdapter::completeConstruction() {
  DCHECK_EQ(priv->state, UserScriptAdapterPrivate::Constructing);

  priv->state = UserScriptAdapterPrivate::Loading;

  if (priv->user_script.url().scheme() != "file") {
    OnScriptLoadFailed();
    return;
  }

  if (!priv->Load()) {
    priv->state = UserScriptAdapterPrivate::FailedLoad;
    OnScriptLoadFailed();
  }
}

} // namespace qt
} // namespace oxide
