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

#include "shared/common/oxide_user_script.h"

#include "qt/core/api/private/oxide_q_user_script_p.h"

OxideQUserScript::OxideQUserScript(QObject* parent) :
    QObject(parent),
    d_ptr(new oxide::qt::QUserScriptPrivate(this)) {}

OxideQUserScript::~OxideQUserScript() {}

void OxideQUserScript::classBegin() {}

void OxideQUserScript::componentComplete() {
  startLoading();
}

void OxideQUserScript::startLoading() {
  Q_D(oxide::qt::QUserScript);

  d->startLoading();
}

OxideQUserScript::State OxideQUserScript::state() const {
  Q_D(const oxide::qt::QUserScript);

  return d->state();
}

QUrl OxideQUserScript::url() const {
  Q_D(const oxide::qt::QUserScript);

  return d->url();
}

void OxideQUserScript::setUrl(const QUrl& url) {
  Q_D(oxide::qt::QUserScript);

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

  d->setUrl(url);
}

bool OxideQUserScript::emulateGreasemonkey() const {
  Q_D(const oxide::qt::QUserScript);

  return d->user_script()->emulate_greasemonkey();
}

void OxideQUserScript::setEmulateGreasemonkey(bool emulate_greasemonkey) {
  Q_D(oxide::qt::QUserScript);

  if (emulate_greasemonkey == emulateGreasemonkey()) {
    return;
  }

  d->user_script()->set_emulate_greasemonkey(emulate_greasemonkey);
  emit scriptPropertyChanged();
}

bool OxideQUserScript::matchAllFrames() const {
  Q_D(const oxide::qt::QUserScript);

  return d->user_script()->match_all_frames();
}

void OxideQUserScript::setMatchAllFrames(bool match_all_frames) {
  Q_D(oxide::qt::QUserScript);

  if (match_all_frames == matchAllFrames()) {
    return;
  }

  d->user_script()->set_match_all_frames(match_all_frames);
  emit scriptPropertyChanged();
}

bool OxideQUserScript::incognitoEnabled() const {
  Q_D(const oxide::qt::QUserScript);

  return d->user_script()->incognito_enabled();
}

void OxideQUserScript::setIncognitoEnabled(bool incognito_enabled) {
  Q_D(oxide::qt::QUserScript);

  if (incognito_enabled == incognitoEnabled()) {
    return;
  }

  d->user_script()->set_incognito_enabled(incognito_enabled);
  emit scriptPropertyChanged();
}

QString OxideQUserScript::worldId() const {
  Q_D(const oxide::qt::QUserScript);

  return QString::fromStdString(d->user_script()->world_id());
}

void OxideQUserScript::setWorldId(const QString& world_id) {
  Q_D(oxide::qt::QUserScript);

  if (world_id == worldId()) {
    return;
  }

  d->user_script()->set_world_id(world_id.toStdString());
  emit scriptPropertyChanged();
}
