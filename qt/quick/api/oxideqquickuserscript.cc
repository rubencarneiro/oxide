// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include "oxideqquickuserscript.h"
#include "oxideqquickuserscript_p.h"

#include "qt/core/glue/oxide_qt_user_script_proxy.h"
#include "qt/quick/oxide_qquick_init.h"

#include "oxideqquickwebcontext_p.h"

struct OxideQQuickUserScriptPrivate::ConstructProps {
  ConstructProps()
      : emulate_greasemonkey(false),
        match_all_frames(false),
        incognito_enabled(false) {}

  bool emulate_greasemonkey;
  bool match_all_frames;
  bool incognito_enabled;
  QUrl context;
};

OxideQQuickUserScriptPrivate::OxideQQuickUserScriptPrivate(
    OxideQQuickUserScript* q)
    : q_ptr(q),
      construct_props_(new ConstructProps()) {}

void OxideQQuickUserScriptPrivate::ScriptLoadFailed() {
  Q_Q(OxideQQuickUserScript);

  emit q->scriptLoadFailed();
}

void OxideQQuickUserScriptPrivate::ScriptLoaded() {
  Q_Q(OxideQQuickUserScript);

  emit q->scriptLoaded();
}

OxideQQuickUserScriptPrivate::~OxideQQuickUserScriptPrivate() {}

// static
OxideQQuickUserScriptPrivate* OxideQQuickUserScriptPrivate::get(
    OxideQQuickUserScript* user_script) {
  return user_script->d_func();
}

void OxideQQuickUserScript::classBegin() {}

void OxideQQuickUserScript::componentComplete() {
  Q_D(OxideQQuickUserScript);

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

  // Script loading uses Chromium's file thread
  oxide::qquick::EnsureChromiumStarted();

  d->proxy_.reset(oxide::qt::UserScriptProxy::create(d, this, d->url_));

  d->proxy_->setEmulateGreasemonkey(
      d->construct_props_->emulate_greasemonkey);
  d->proxy_->setMatchAllFrames(d->construct_props_->match_all_frames);
  d->proxy_->setIncognitoEnabled(d->construct_props_->incognito_enabled);
  d->proxy_->setContext(d->construct_props_->context);

  d->construct_props_.reset();
}

OxideQQuickUserScript::OxideQQuickUserScript(QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQQuickUserScriptPrivate(this)) {
}

OxideQQuickUserScript::~OxideQQuickUserScript() {
  Q_D(OxideQQuickUserScript);

  emit d->willBeDeleted();
}

QUrl OxideQQuickUserScript::url() const {
  Q_D(const OxideQQuickUserScript);

  return d->url_;
}

void OxideQQuickUserScript::setUrl(const QUrl& url) {
  Q_D(OxideQQuickUserScript);

  if (d->proxy_) {
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

  if (!d->proxy_) {
    return d->construct_props_->emulate_greasemonkey;
  }

  return d->proxy_->emulateGreasemonkey();
}

void OxideQQuickUserScript::setEmulateGreasemonkey(bool emulate_greasemonkey) {
  Q_D(OxideQQuickUserScript);

  if (emulate_greasemonkey == emulateGreasemonkey()) {
    return;
  }

  if (!d->proxy_) {
    d->construct_props_->emulate_greasemonkey = emulate_greasemonkey;
  } else {
    d->proxy_->setEmulateGreasemonkey(emulate_greasemonkey);
  }

  emit scriptPropertyChanged();
}

bool OxideQQuickUserScript::matchAllFrames() const {
  Q_D(const OxideQQuickUserScript);

  if (!d->proxy_) {
    return d->construct_props_->match_all_frames;
  }

  return d->proxy_->matchAllFrames();
}

void OxideQQuickUserScript::setMatchAllFrames(bool match_all_frames) {
  Q_D(OxideQQuickUserScript);

  if (match_all_frames == matchAllFrames()) {
    return;
  }

  if (!d->proxy_) {
    d->construct_props_->match_all_frames = match_all_frames;
  } else {
    d->proxy_->setMatchAllFrames(match_all_frames);
  }

  emit scriptPropertyChanged();
}

bool OxideQQuickUserScript::incognitoEnabled() const {
  Q_D(const OxideQQuickUserScript);

  if (!d->proxy_) {
    return d->construct_props_->incognito_enabled;
  }

  return d->proxy_->incognitoEnabled();
}

void OxideQQuickUserScript::setIncognitoEnabled(bool incognito_enabled) {
  Q_D(OxideQQuickUserScript);

  if (incognito_enabled == incognitoEnabled()) {
    return;
  }

  if (!d->proxy_) {
    d->construct_props_->incognito_enabled = incognito_enabled;
  } else {
    d->proxy_->setIncognitoEnabled(incognito_enabled);
  }

  emit scriptPropertyChanged();
}

QUrl OxideQQuickUserScript::context() const {
  Q_D(const OxideQQuickUserScript);

  if (!d->proxy_) {
    return d->construct_props_->context;
  }

  return d->proxy_->context();
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

  if (!d->proxy_) {
    d->construct_props_->context = context;
  } else {
    d->proxy_->setContext(context);
  }

  emit scriptPropertyChanged();
}
