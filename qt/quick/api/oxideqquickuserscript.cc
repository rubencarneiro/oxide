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

#include "oxideqquickuserscript_p.h"
#include "oxideqquickuserscript_p_p.h"

#include <QtDebug>

#include "shared/common/oxide_user_script.h"

OxideQQuickUserScript::OxideQQuickUserScript(QObject* parent) :
    QObject(parent),
    d_ptr(new oxide::qt::QQuickUserScriptPrivate(this)) {}

OxideQQuickUserScript::~OxideQQuickUserScript() {}

void OxideQQuickUserScript::classBegin() {}

void OxideQQuickUserScript::componentComplete() {
  startLoading();
}

void OxideQQuickUserScript::startLoading() {
  Q_D(oxide::qt::QQuickUserScript);

  d->startLoading();
}

OxideQQuickUserScript::State OxideQQuickUserScript::state() const {
  Q_D(const oxide::qt::QQuickUserScript);

  return d->state();
}

QUrl OxideQQuickUserScript::url() const {
  Q_D(const oxide::qt::QQuickUserScript);

  return d->url();
}

void OxideQQuickUserScript::setUrl(const QUrl& url) {
  Q_D(oxide::qt::QQuickUserScript);

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

bool OxideQQuickUserScript::emulateGreasemonkey() const {
  Q_D(const oxide::qt::QQuickUserScript);

  return d->user_script()->emulate_greasemonkey();
}

void OxideQQuickUserScript::setEmulateGreasemonkey(bool emulate_greasemonkey) {
  Q_D(oxide::qt::QQuickUserScript);

  if (emulate_greasemonkey == emulateGreasemonkey()) {
    return;
  }

  d->user_script()->set_emulate_greasemonkey(emulate_greasemonkey);
  emit scriptPropertyChanged();
}

bool OxideQQuickUserScript::matchAllFrames() const {
  Q_D(const oxide::qt::QQuickUserScript);

  return d->user_script()->match_all_frames();
}

void OxideQQuickUserScript::setMatchAllFrames(bool match_all_frames) {
  Q_D(oxide::qt::QQuickUserScript);

  if (match_all_frames == matchAllFrames()) {
    return;
  }

  d->user_script()->set_match_all_frames(match_all_frames);
  emit scriptPropertyChanged();
}

bool OxideQQuickUserScript::incognitoEnabled() const {
  Q_D(const oxide::qt::QQuickUserScript);

  return d->user_script()->incognito_enabled();
}

void OxideQQuickUserScript::setIncognitoEnabled(bool incognito_enabled) {
  Q_D(oxide::qt::QQuickUserScript);

  if (incognito_enabled == incognitoEnabled()) {
    return;
  }

  d->user_script()->set_incognito_enabled(incognito_enabled);
  emit scriptPropertyChanged();
}

QString OxideQQuickUserScript::worldId() const {
  Q_D(const oxide::qt::QQuickUserScript);

  return QString::fromStdString(d->user_script()->world_id());
}

void OxideQQuickUserScript::setWorldId(const QString& world_id) {
  Q_D(oxide::qt::QQuickUserScript);

  if (world_id == worldId()) {
    return;
  }

  d->user_script()->set_world_id(world_id.toStdString());
  emit scriptPropertyChanged();
}
