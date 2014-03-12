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

#include "oxideqquickwebcontext_p.h"

void OxideQQuickUserScriptPrivate::OnScriptLoadFailed() {
  Q_Q(OxideQQuickUserScript);

  emit q->scriptLoadFailed();
}

void OxideQQuickUserScriptPrivate::OnScriptLoaded() {
  Q_Q(OxideQQuickUserScript);

  emit q->scriptLoaded();
}

OxideQQuickUserScriptPrivate::OxideQQuickUserScriptPrivate(
    OxideQQuickUserScript* q) :
    oxide::qt::UserScriptAdapter(q) {}

// static
OxideQQuickUserScriptPrivate* OxideQQuickUserScriptPrivate::get(
    OxideQQuickUserScript* user_script) {
  return user_script->d_func();
}

OxideQQuickUserScript::OxideQQuickUserScript(QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQQuickUserScriptPrivate(this)) {
  // Script loading uses Chromium's file thread
  OxideQQuickWebContext::ensureChromiumStarted();
}

OxideQQuickUserScript::~OxideQQuickUserScript() {
  emit scriptWillBeDeleted();
}

void OxideQQuickUserScript::classBegin() {}

void OxideQQuickUserScript::componentComplete() {
  Q_D(OxideQQuickUserScript);

  d->completeConstruction();
}

QUrl OxideQQuickUserScript::url() const {
  Q_D(const OxideQQuickUserScript);

  return d->url();
}

void OxideQQuickUserScript::setUrl(const QUrl& url) {
  Q_D(OxideQQuickUserScript);

  d->setUrl(url);
}

bool OxideQQuickUserScript::privateInjectedInMainWorld() const {
  Q_D(const OxideQQuickUserScript);

  return d->privateInjectedInMainWorld();
}

void OxideQQuickUserScript::setPrivateInjectedInMainWorld(bool in_main_world) {
  Q_D(OxideQQuickUserScript);

  if (in_main_world == privateInjectedInMainWorld()) {
    return;
  }

  d->setPrivateInjectedInMainWorld(in_main_world);
  emit scriptPropertyChanged();
}

bool OxideQQuickUserScript::emulateGreasemonkey() const {
  Q_D(const OxideQQuickUserScript);

  return d->emulateGreasemonkey();
}

void OxideQQuickUserScript::setEmulateGreasemonkey(bool emulate_greasemonkey) {
  Q_D(OxideQQuickUserScript);

  if (emulate_greasemonkey == emulateGreasemonkey()) {
    return;
  }

  d->setEmulateGreasemonkey(emulate_greasemonkey);
  emit scriptPropertyChanged();
}

bool OxideQQuickUserScript::matchAllFrames() const {
  Q_D(const OxideQQuickUserScript);

  return d->matchAllFrames();
}

void OxideQQuickUserScript::setMatchAllFrames(bool match_all_frames) {
  Q_D(OxideQQuickUserScript);

  if (match_all_frames == matchAllFrames()) {
    return;
  }

  d->setMatchAllFrames(match_all_frames);
  emit scriptPropertyChanged();
}

bool OxideQQuickUserScript::incognitoEnabled() const {
  Q_D(const OxideQQuickUserScript);

  return d->incognitoEnabled();
}

void OxideQQuickUserScript::setIncognitoEnabled(bool incognito_enabled) {
  Q_D(OxideQQuickUserScript);

  if (incognito_enabled == incognitoEnabled()) {
    return;
  }

  d->setIncognitoEnabled(incognito_enabled);
  emit scriptPropertyChanged();
}

QUrl OxideQQuickUserScript::context() const {
  Q_D(const OxideQQuickUserScript);

  return d->context();
}

void OxideQQuickUserScript::setContext(const QUrl& context) {
  Q_D(OxideQQuickUserScript);

  if (context == this->context()) {
    return;
  }

  d->setContext(context);
  emit scriptPropertyChanged();
}
