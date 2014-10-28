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

#include <QDebug>
#include <string>

#include "base/files/file_path.h"
#include "url/gurl.h"

#include "qt/core/browser/oxide_qt_user_script.h"
#include "shared/common/oxide_user_script.h"

namespace oxide {
namespace qt {

UserScriptAdapter::UserScriptAdapter(QObject* q) :
    AdapterBase(q),
    script_(new UserScript(this)) {}

UserScriptAdapter::~UserScriptAdapter() {}

bool UserScriptAdapter::emulateGreasemonkey() const {
  return script_->GetEmulateGreasemonkey();
}

void UserScriptAdapter::setEmulateGreasemonkey(bool emulate) {
  script_->SetEmulateGreasemonkey(emulate);
}

bool UserScriptAdapter::matchAllFrames() const {
  return script_->GetMatchAllFrames();
}

void UserScriptAdapter::setMatchAllFrames(bool match) {
  script_->SetMatchAllFrames(match);
}

bool UserScriptAdapter::incognitoEnabled() const {
  return script_->GetIncognitoEnabled();
}

void UserScriptAdapter::setIncognitoEnabled(bool enabled) {
  script_->SetIncognitoEnabled(enabled);
}

QUrl UserScriptAdapter::context() const {
  return QUrl(QString::fromStdString(script_->GetContext().spec()));
}

void UserScriptAdapter::setContext(const QUrl& context) {
  if (!context.isValid()) {
    qWarning() << "UserScript context must be set to a valid URL";
    return;
  }

  script_->SetContext(GURL(context.toString().toStdString()));
}

void UserScriptAdapter::init(const QUrl& url) {
  if (!url.isValid()) {
    qWarning() << "UserScript url must be set to a valid URL";
    OnScriptLoadFailed();
    return;
  }

  if (!url.isLocalFile()) {
    qWarning() << "UserScript url must be set to a local file";
    OnScriptLoadFailed();
    return;
  }

  script_->Init(base::FilePath(url.toLocalFile().toStdString()));
}

} // namespace qt
} // namespace oxide
