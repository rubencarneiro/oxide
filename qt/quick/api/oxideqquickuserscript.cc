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

#include "qt/quick/oxide_qquick_init.h"

#include "oxideqquickwebcontext_p_p.h"

OXIDE_Q_IMPL_PROXY_HANDLE_CONVERTER(OxideQQuickUserScript,
                                    oxide::qt::UserScriptProxyHandle);

void OxideQQuickUserScriptPrivate::ScriptLoadFailed() {
  Q_Q(OxideQQuickUserScript);

  emit q->scriptLoadFailed();
}

void OxideQQuickUserScriptPrivate::ScriptLoaded() {
  Q_Q(OxideQQuickUserScript);

  emit q->scriptLoaded();
}

OxideQQuickUserScriptPrivate::OxideQQuickUserScriptPrivate(
    OxideQQuickUserScript* q)
    : oxide::qt::UserScriptProxyHandle(
        oxide::qt::UserScriptProxy::create(this), q),
      constructed_(false) {}

// static
OxideQQuickUserScriptPrivate* OxideQQuickUserScriptPrivate::get(
    OxideQQuickUserScript* user_script) {
  return user_script->d_func();
}

OxideQQuickUserScript::OxideQQuickUserScript(QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQQuickUserScriptPrivate(this)) {
  // Script loading uses Chromium's file thread
  oxide::qquick::EnsureChromiumStarted();
}

OxideQQuickUserScript::~OxideQQuickUserScript() {
  Q_D(OxideQQuickUserScript);

  emit d->willBeDeleted();
}

void OxideQQuickUserScript::classBegin() {}

void OxideQQuickUserScript::componentComplete() {
  Q_D(OxideQQuickUserScript);

  d->constructed_ = true;

  if (!d->url_.isValid()) {
    qWarning() <<
        "OxideQQuickUserScript::componentComplete: url must be set to a "
        "valid URL";
    emit scriptLoadFailed();
    return;
  }

  if (!d->url_.isLocalFile()) {
    qWarning() <<
        "OxideQQuickUserScript::componentComplete: url must be set to a "
        "local file";
    emit scriptLoadFailed();
    return;
  }

  d->proxy()->init(d->url_);
}

QUrl OxideQQuickUserScript::url() const {
  Q_D(const OxideQQuickUserScript);

  return d->url_;
}

void OxideQQuickUserScript::setUrl(const QUrl& url) {
  Q_D(OxideQQuickUserScript);

  if (d->constructed_) {
    qWarning() << "OxideQQuickUserScript: url is a construct-only parameter";
    return;
  }

  if (url == d->url_) {
    return;
  }

  d->url_ = url;
  emit scriptPropertyChanged();
}

bool OxideQQuickUserScript::emulateGreasemonkey() const {
  Q_D(const OxideQQuickUserScript);

  return d->proxy()->emulateGreasemonkey();
}

void OxideQQuickUserScript::setEmulateGreasemonkey(bool emulate_greasemonkey) {
  Q_D(OxideQQuickUserScript);

  if (emulate_greasemonkey == emulateGreasemonkey()) {
    return;
  }

  d->proxy()->setEmulateGreasemonkey(emulate_greasemonkey);
  emit scriptPropertyChanged();
}

bool OxideQQuickUserScript::matchAllFrames() const {
  Q_D(const OxideQQuickUserScript);

  return d->proxy()->matchAllFrames();
}

void OxideQQuickUserScript::setMatchAllFrames(bool match_all_frames) {
  Q_D(OxideQQuickUserScript);

  if (match_all_frames == matchAllFrames()) {
    return;
  }

  d->proxy()->setMatchAllFrames(match_all_frames);
  emit scriptPropertyChanged();
}

bool OxideQQuickUserScript::incognitoEnabled() const {
  Q_D(const OxideQQuickUserScript);

  return d->proxy()->incognitoEnabled();
}

void OxideQQuickUserScript::setIncognitoEnabled(bool incognito_enabled) {
  Q_D(OxideQQuickUserScript);

  if (incognito_enabled == incognitoEnabled()) {
    return;
  }

  d->proxy()->setIncognitoEnabled(incognito_enabled);
  emit scriptPropertyChanged();
}

QUrl OxideQQuickUserScript::context() const {
  Q_D(const OxideQQuickUserScript);

  return d->proxy()->context();
}

void OxideQQuickUserScript::setContext(const QUrl& context) {
  Q_D(OxideQQuickUserScript);

  if (context == this->context()) {
    return;
  }

  if (!context.isValid()) {
    qWarning() << "OxideQQuickUserScript: context must be a valid URL";
    return;
  }

  d->proxy()->setContext(context);
  emit scriptPropertyChanged();
}
