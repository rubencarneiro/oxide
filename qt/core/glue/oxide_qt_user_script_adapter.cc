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

#include "url/gurl.h"

#include "shared/common/oxide_user_script.h"

#include "qt/core/glue/private/oxide_qt_user_script_adapter_p.h"

namespace oxide {
namespace qt {

UserScriptAdapter::UserScriptAdapter() :
    priv_(UserScriptAdapterPrivate::Create(this)) {}

UserScriptAdapter::~UserScriptAdapter() {}

UserScriptAdapter::State UserScriptAdapter::state() const {
  return priv_->state();
}

QUrl UserScriptAdapter::url() const {
  return QUrl(QString::fromStdString(priv_->user_script().url().spec()));
}

void UserScriptAdapter::setUrl(const QUrl& url) {
  priv_->user_script().set_url(GURL(url.toString().toStdString()));
}

bool UserScriptAdapter::emulateGreasemonkey() const {
  return priv_->user_script().emulate_greasemonkey();
}

void UserScriptAdapter::setEmulateGreasemonkey(bool emulate) {
  priv_->user_script().set_emulate_greasemonkey(emulate);
}

bool UserScriptAdapter::matchAllFrames() const {
  return priv_->user_script().match_all_frames();
}

void UserScriptAdapter::setMatchAllFrames(bool match) {
  priv_->user_script().set_match_all_frames(match);
}

bool UserScriptAdapter::incognitoEnabled() const {
  return priv_->user_script().incognito_enabled();
}

void UserScriptAdapter::setIncognitoEnabled(bool enabled) {
  priv_->user_script().set_incognito_enabled(enabled);
}

QString UserScriptAdapter::worldId() const {
  return QString::fromStdString(priv_->user_script().world_id());
}

void UserScriptAdapter::setWorldId(const QString& id) {
  priv_->user_script().set_world_id(id.toStdString());
}

void UserScriptAdapter::startLoading() {
  priv_->StartLoading();
}

} // namespace qt
} // namespace oxide
