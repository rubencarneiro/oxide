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

#include "oxideqwebpreferences.h"
#include "oxideqwebpreferences_p.h"

#include <QtDebug>

OxideQWebPreferences::~OxideQWebPreferences() {}

OxideQWebPreferences::OxideQWebPreferences(QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQWebPreferencesPrivate(this)) {}

QString OxideQWebPreferences::standardFontFamily() const {
  Q_D(const OxideQWebPreferences);
  return QString::fromStdString(d->preferences.StandardFontFamily());
}

void OxideQWebPreferences::setStandardFontFamily(const QString& font) {
  Q_D(OxideQWebPreferences);

  if (font == standardFontFamily()) {
    return;
  }

  d->preferences.SetStandardFontFamily(font.toStdString());
  Q_EMIT standardFontFamilyChanged();
}

QString OxideQWebPreferences::fixedFontFamily() const {
  Q_D(const OxideQWebPreferences);
  return QString::fromStdString(d->preferences.FixedFontFamily());
}

void OxideQWebPreferences::setFixedFontFamily(const QString& font) {
  Q_D(OxideQWebPreferences);

  if (font == fixedFontFamily()) {
    return;
  }

  d->preferences.SetFixedFontFamily(font.toStdString());
  Q_EMIT fixedFontFamilyChanged();
}

QString OxideQWebPreferences::serifFontFamily() const {
  Q_D(const OxideQWebPreferences);
  return QString::fromStdString(d->preferences.SerifFontFamily());
}

void OxideQWebPreferences::setSerifFontFamily(const QString& font) {
  Q_D(OxideQWebPreferences);

  if (font == serifFontFamily()) {
    return;
  }

  d->preferences.SetSerifFontFamily(font.toStdString());
  Q_EMIT serifFontFamilyChanged();
}

QString OxideQWebPreferences::sansSerifFontFamily() const {
  Q_D(const OxideQWebPreferences);
  return QString::fromStdString(d->preferences.SansSerifFontFamily());
}

void OxideQWebPreferences::setSansSerifFontFamily(const QString& font) {
  Q_D(OxideQWebPreferences);

  if (font == sansSerifFontFamily()) {
    return;
  }

  d->preferences.SetSansSerifFontFamily(font.toStdString());
  Q_EMIT sansSerifFontFamilyChanged();
}

QString OxideQWebPreferences::defaultEncoding() const {
  Q_D(const OxideQWebPreferences);
  return QString::fromStdString(d->preferences.default_encoding());
}

void OxideQWebPreferences::setDefaultEncoding(const QString& encoding) {
  Q_D(OxideQWebPreferences);

  if (encoding == defaultEncoding()) {
    return;
  }

  d->preferences.SetDefaultEncoding(encoding.toStdString());
  Q_EMIT defaultEncodingChanged();
}

unsigned OxideQWebPreferences::defaultFontSize() const {
  Q_D(const OxideQWebPreferences);
  return d->preferences.default_font_size();
}

void OxideQWebPreferences::setDefaultFontSize(unsigned size) {
  Q_D(OxideQWebPreferences);

  if (size == defaultFontSize()) {
    return;
  }

  d->preferences.SetDefaultFontSize(size);
  Q_EMIT defaultFontSizeChanged();
}

unsigned OxideQWebPreferences::defaultFixedFontSize() const {
  Q_D(const OxideQWebPreferences);
  return d->preferences.default_fixed_font_size();
}

void OxideQWebPreferences::setDefaultFixedFontSize(unsigned size) {
  Q_D(OxideQWebPreferences);

  if (size == defaultFixedFontSize()) {
    return;
  }

  d->preferences.SetDefaultFixedFontSize(size);
  Q_EMIT defaultFixedFontSizeChanged();
}

unsigned OxideQWebPreferences::minimumFontSize() const {
  Q_D(const OxideQWebPreferences);
  return d->preferences.minimum_font_size();
}

void OxideQWebPreferences::setMinimumFontSize(unsigned size) {
  Q_D(OxideQWebPreferences);

  if (size == minimumFontSize()) {
    return;
  }

  d->preferences.SetMinimumFontSize(size);
  Q_EMIT minimumFontSizeChanged();
}

#define BOOLEAN_PREF_IMPL(getter, setter, attr) \
  bool OxideQWebPreferences::getter() const { \
    Q_D(const OxideQWebPreferences); \
    return d->preferences.TestAttribute( \
        oxide::WebPreferences::ATTR_##attr); \
  } \
\
  void OxideQWebPreferences::setter(bool val) { \
    Q_D(OxideQWebPreferences); \
    if (val == getter()) { \
      return; \
    } \
    d->preferences.SetAttribute( \
        oxide::WebPreferences::ATTR_##attr, val); \
    Q_EMIT getter##Changed(); \
  }

#define DEPRECATED_BOOLEAN_PREF_IMPL(getter, setter, value) \
  bool OxideQWebPreferences::getter() const { \
    return value; \
  } \
\
  void OxideQWebPreferences::setter(bool val) { \
    qWarning() << #getter " is deprecated and has no effect"; \
  }

BOOLEAN_PREF_IMPL(remoteFontsEnabled, setRemoteFontsEnabled, REMOTE_FONTS_ENABLED)
BOOLEAN_PREF_IMPL(javascriptEnabled, setJavascriptEnabled, JAVASCRIPT_ENABLED)
BOOLEAN_PREF_IMPL(allowScriptsToCloseWindows, setAllowScriptsToCloseWindows, ALLOW_SCRIPTS_TO_CLOSE_WINDOWS)
BOOLEAN_PREF_IMPL(javascriptCanAccessClipboard, setJavascriptCanAccessClipboard, JAVASCRIPT_CAN_ACCESS_CLIPBOARD)
BOOLEAN_PREF_IMPL(hyperlinkAuditingEnabled, setHyperlinkAuditingEnabled, HYPERLINK_AUDITING_ENABLED)
BOOLEAN_PREF_IMPL(allowUniversalAccessFromFileUrls, setAllowUniversalAccessFromFileUrls, ALLOW_UNIVERSAL_ACCESS_FROM_FILE_URLS)
BOOLEAN_PREF_IMPL(allowFileAccessFromFileUrls, setAllowFileAccessFromFileUrls, ALLOW_FILE_ACCESS_FROM_FILE_URLS)
BOOLEAN_PREF_IMPL(canDisplayInsecureContent, setCanDisplayInsecureContent, CAN_DISPLAY_INSECURE_CONTENT)
BOOLEAN_PREF_IMPL(canRunInsecureContent, setCanRunInsecureContent, CAN_RUN_INSECURE_CONTENT)
BOOLEAN_PREF_IMPL(passwordEchoEnabled, setPasswordEchoEnabled, PASSWORD_ECHO_ENABLED)
BOOLEAN_PREF_IMPL(loadsImagesAutomatically, setLoadsImagesAutomatically, LOADS_IMAGES_AUTOMATICALLY)
BOOLEAN_PREF_IMPL(shrinksStandaloneImagesToFit, setShrinksStandaloneImagesToFit, SHRINKS_STANDALONE_IMAGES_TO_FIT)
BOOLEAN_PREF_IMPL(textAreasAreResizable, setTextAreasAreResizable, TEXT_AREAS_ARE_RESIZABLE)
BOOLEAN_PREF_IMPL(localStorageEnabled, setLocalStorageEnabled, LOCAL_STORAGE_ENABLED)
DEPRECATED_BOOLEAN_PREF_IMPL(databasesEnabled, setDatabasesEnabled, true)
BOOLEAN_PREF_IMPL(appCacheEnabled, setAppCacheEnabled, APP_CACHE_ENABLED)
BOOLEAN_PREF_IMPL(tabsToLinks, setTabsToLinks, TABS_TO_LINKS)
BOOLEAN_PREF_IMPL(caretBrowsingEnabled, setCaretBrowsingEnabled, CARET_BROWSING_ENABLED)
DEPRECATED_BOOLEAN_PREF_IMPL(touchEnabled, setTouchEnabled, true)
