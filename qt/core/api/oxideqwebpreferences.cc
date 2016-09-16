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

#include "qt/core/browser/oxide_qt_web_preferences.h"

#include "oxideqglobal_p.h"

OxideQWebPreferencesPrivate::OxideQWebPreferencesPrivate(
    OxideQWebPreferences* q)
    : preferences_(q) {}

OxideQWebPreferencesPrivate::~OxideQWebPreferencesPrivate() {}

// static
OxideQWebPreferencesPrivate* OxideQWebPreferencesPrivate::get(OxideQWebPreferences* q) {
  return q->d_func();
}

/*!
\class OxideQWebPreferences
\inmodule OxideQtCore
\inheaderfile oxideqwebpreferences.h

\brief Web settings

OxideQWebPreferences contains a collection of settings for a webview.
*/

/*!
Destroy this preferences instance.
*/

OxideQWebPreferences::~OxideQWebPreferences() {}

/*!
Construct a new preferences instance and make it a child of \a{parent}.
*/

OxideQWebPreferences::OxideQWebPreferences(QObject* parent)
    : QObject(parent),
      d_ptr(new OxideQWebPreferencesPrivate(this)) {}

/*!
\property OxideQWebPreferences::standardFontFamily

The default standard font family for characters from the Unicode \e{Common}
script.

The default value is dependent on the system's font configuration.
*/

QString OxideQWebPreferences::standardFontFamily() const {
  Q_D(const OxideQWebPreferences);
  return QString::fromStdString(d->preferences_.StandardFontFamily());
}

void OxideQWebPreferences::setStandardFontFamily(const QString& font) {
  Q_D(OxideQWebPreferences);

  if (font == standardFontFamily()) {
    return;
  }

  d->preferences_.SetStandardFontFamily(font.toStdString());
  Q_EMIT standardFontFamilyChanged();
}

/*!
\property OxideQWebPreferences::fixedFontFamily

The default fixed font family for characters from the Unicode \e{Common} script.

The default value is dependent on the system's font configuration.
*/

QString OxideQWebPreferences::fixedFontFamily() const {
  Q_D(const OxideQWebPreferences);
  return QString::fromStdString(d->preferences_.FixedFontFamily());
}

void OxideQWebPreferences::setFixedFontFamily(const QString& font) {
  Q_D(OxideQWebPreferences);

  if (font == fixedFontFamily()) {
    return;
  }

  d->preferences_.SetFixedFontFamily(font.toStdString());
  Q_EMIT fixedFontFamilyChanged();
}

/*!
\property OxideQWebPreferences::serifFontFamily

The default Serif font family for characters from the Unicode \e{Common} script.

The default value is dependent on the system's font configuration.
*/

QString OxideQWebPreferences::serifFontFamily() const {
  Q_D(const OxideQWebPreferences);
  return QString::fromStdString(d->preferences_.SerifFontFamily());
}

void OxideQWebPreferences::setSerifFontFamily(const QString& font) {
  Q_D(OxideQWebPreferences);

  if (font == serifFontFamily()) {
    return;
  }

  d->preferences_.SetSerifFontFamily(font.toStdString());
  Q_EMIT serifFontFamilyChanged();
}

/*!
\property OxideQWebPreferences::sanSerifFontFamily

The default Sans Serif font family for characters from the Unicode \e{Common}
script.

The default value is dependent on the system's font configuration.
*/

QString OxideQWebPreferences::sansSerifFontFamily() const {
  Q_D(const OxideQWebPreferences);
  return QString::fromStdString(d->preferences_.SansSerifFontFamily());
}

void OxideQWebPreferences::setSansSerifFontFamily(const QString& font) {
  Q_D(OxideQWebPreferences);

  if (font == sansSerifFontFamily()) {
    return;
  }

  d->preferences_.SetSansSerifFontFamily(font.toStdString());
  Q_EMIT sansSerifFontFamilyChanged();
}

/*!
\property OxideQWebPreferences::defaultEncoding

The default character set, used for pages that don't define a character set.
*/

QString OxideQWebPreferences::defaultEncoding() const {
  Q_D(const OxideQWebPreferences);
  return QString::fromStdString(d->preferences_.default_encoding());
}

void OxideQWebPreferences::setDefaultEncoding(const QString& encoding) {
  Q_D(OxideQWebPreferences);

  if (encoding == defaultEncoding()) {
    return;
  }

  d->preferences_.SetDefaultEncoding(encoding.toStdString());
  Q_EMIT defaultEncodingChanged();
}

/*!
\property OxideQWebPreferences::defaultFontSize

The default font size in points.
*/

unsigned OxideQWebPreferences::defaultFontSize() const {
  Q_D(const OxideQWebPreferences);
  return d->preferences_.default_font_size();
}

void OxideQWebPreferences::setDefaultFontSize(unsigned size) {
  Q_D(OxideQWebPreferences);

  if (size == defaultFontSize()) {
    return;
  }

  d->preferences_.SetDefaultFontSize(size);
  Q_EMIT defaultFontSizeChanged();
}

/*!
\property OxideQWebPreferences::defaultFixedFontSize

The default fixed font size in points.
*/

unsigned OxideQWebPreferences::defaultFixedFontSize() const {
  Q_D(const OxideQWebPreferences);
  return d->preferences_.default_fixed_font_size();
}

void OxideQWebPreferences::setDefaultFixedFontSize(unsigned size) {
  Q_D(OxideQWebPreferences);

  if (size == defaultFixedFontSize()) {
    return;
  }

  d->preferences_.SetDefaultFixedFontSize(size);
  Q_EMIT defaultFixedFontSizeChanged();
}

/*!
\property OxideQWebPreferences::minimumFontSize

The minimum font size in points.
*/

unsigned OxideQWebPreferences::minimumFontSize() const {
  Q_D(const OxideQWebPreferences);
  return d->preferences_.minimum_font_size();
}

void OxideQWebPreferences::setMinimumFontSize(unsigned size) {
  Q_D(OxideQWebPreferences);

  if (size == minimumFontSize()) {
    return;
  }

  d->preferences_.SetMinimumFontSize(size);
  Q_EMIT minimumFontSizeChanged();
}

#define BOOLEAN_PREF_IMPL(getter, setter, attr) \
  bool OxideQWebPreferences::getter() const { \
    Q_D(const OxideQWebPreferences); \
    return d->preferences_.TestAttribute( \
        oxide::WebPreferences::ATTR_##attr); \
  } \
\
  void OxideQWebPreferences::setter(bool val) { \
    Q_D(OxideQWebPreferences); \
    if (val == getter()) { \
      return; \
    } \
    d->preferences_.SetAttribute( \
        oxide::WebPreferences::ATTR_##attr, val); \
    Q_EMIT getter##Changed(); \
  }

#define DEPRECATED_BOOLEAN_PREF_IMPL(getter, setter, value) \
  bool OxideQWebPreferences::getter() const { \
    return value; \
  } \
\
  void OxideQWebPreferences::setter(bool val) { \
    WARN_DEPRECATED_API_USAGE() << \
        "OxideQWebPreferences: " << #getter << \
        " is deprecated and has no effect"; \
  }

/*!
\property OxideQWebPreferences::remoteFontsEnabled

Whether support for remote web fonts is enabled. The default is true.
*/

BOOLEAN_PREF_IMPL(remoteFontsEnabled, setRemoteFontsEnabled, REMOTE_FONTS_ENABLED)

/*!
\property OxideQWebPreferences::javascriptEnabled

Whether JavaScript is enabled. The default is true.
*/

BOOLEAN_PREF_IMPL(javascriptEnabled, setJavascriptEnabled, JAVASCRIPT_ENABLED)

/*!
\property OxideQWebPreferences::allowScriptsToCloseWindows

Whether web content can request to close a window via \e{window.close}. The
default is false.

\note Web content can always request to close windows that were opened by it via
\e{window.open()}, regardless of this preference.
*/

BOOLEAN_PREF_IMPL(allowScriptsToCloseWindows, setAllowScriptsToCloseWindows, ALLOW_SCRIPTS_TO_CLOSE_WINDOWS)

/*!
\property OxideQWebPreferences::javascriptCanAccessClipboard

Whether javascript can use the various clipboard related editing commands via
\e{document.execCommand()}. The default is false.
*/

BOOLEAN_PREF_IMPL(javascriptCanAccessClipboard, setJavascriptCanAccessClipboard, JAVASCRIPT_CAN_ACCESS_CLIPBOARD)

/*!
\property OxideQWebPreferences::hyperlinkAuditingEnabled

Whether to notify the URLs specified by the \e{ping} attribute when a user
clicks on an \e{anchor} element. The default is true.
*/

BOOLEAN_PREF_IMPL(hyperlinkAuditingEnabled, setHyperlinkAuditingEnabled, HYPERLINK_AUDITING_ENABLED)

/*!
\property OxideQWebPreferences::allowUniversalAccessFromFileUrls

Whether to disable same-origin restrictions for pages loaded via file: URLs. The
default is false.

\note This is a dangerous option and should generally not be used in production
code. Application developers should think carefully before enabling this option
to ensure that they are fully aware of its consequences. This option should
never be enabled in web browsers.
*/

BOOLEAN_PREF_IMPL(allowUniversalAccessFromFileUrls, setAllowUniversalAccessFromFileUrls, ALLOW_UNIVERSAL_ACCESS_FROM_FILE_URLS)

/*!
\property OxideQWebPreferences::allowFileAccessFromFileUrls

Whether to disable same-origin restrictions between file: URLs. The default is
false.

By default, unique file: URLs are treated as unique origins for the purposes of
the Same-origin policy. Enabling this causes all file: URLs to be treated as a
single origin.

\note This is a dangerous option and should generally not be used in production
code. Application developers should think carefully before enabling this option
to ensure that they are fully aware of its consequences. This option should
never be enabled in web browsers.
*/

BOOLEAN_PREF_IMPL(allowFileAccessFromFileUrls, setAllowFileAccessFromFileUrls, ALLOW_FILE_ACCESS_FROM_FILE_URLS)

/*!
\property OxideQWebPreferences::canDisplayInsecureContent

Whether to allow the display of passive mixed content. The default is true.

Passive mixed content includes images and videos loaded over an insecure
connection in to a page that was loaded over a secure connection. Passive mixed
content can't access other parts of a web page or change the behaviour of it,
but an attacker could replace the content a user sees or otherwise infer the
user's browsing habits because the connections aren't private.
*/

BOOLEAN_PREF_IMPL(canDisplayInsecureContent, setCanDisplayInsecureContent, CAN_DISPLAY_INSECURE_CONTENT)

/*!
\property OxideQWebPreferences::canRunInsecureContent

Whether to allow the execution of active mixed content. The default is false.

Active mixed content includes CSS and scripts loaded over an insecure connection
in to a page that was loaded over a secure connection. Because active mixed
content can change the behaviour of a page and steal sensitive information, it
compromises the security of a page entirely.
*/

BOOLEAN_PREF_IMPL(canRunInsecureContent, setCanRunInsecureContent, CAN_RUN_INSECURE_CONTENT)

/*!
\property OxideQWebPreferences::passwordEchoEnabled

Whether password echo is enabled. The default is false.

When password echo is enabled, actual characters are displayed when a user
inputs text in to an \e{<input type="password">} element.
*/

BOOLEAN_PREF_IMPL(passwordEchoEnabled, setPasswordEchoEnabled, PASSWORD_ECHO_ENABLED)

/*!
\property OxideQWebPreferences::loadsImagesAutomatically

Whether to load images automatically when a page loads. The default is true.

If this is set to false, images aren't loaded when a page loads, and image
elements are replaced by a placeholder.
*/

BOOLEAN_PREF_IMPL(loadsImagesAutomatically, setLoadsImagesAutomatically, LOADS_IMAGES_AUTOMATICALLY)

/*!
\property OxideQWebPreferences::shrinksStandaloneImagesToFit
\deprecated

Always true. Because of a bug, this preference has never had any effect.
*/

DEPRECATED_BOOLEAN_PREF_IMPL(shrinksStandaloneImagesToFit, setShrinksStandaloneImagesToFit, true)

/*!
\property OxideQWebPreferences::textAreasAreResizable

Whether \e{<textarea>} elements are resizable. The default is true.
*/

BOOLEAN_PREF_IMPL(textAreasAreResizable, setTextAreasAreResizable, TEXT_AREAS_ARE_RESIZABLE)

/*!
\property OxideQWebPreferences::localStorageEnabled

Whether the DOM local storage API is enabled. The default is false.
*/

BOOLEAN_PREF_IMPL(localStorageEnabled, setLocalStorageEnabled, LOCAL_STORAGE_ENABLED)

/*!
\property OxideQWebPreferences::databasesEnabled
\deprecated

Always true. It is not possible to disable Web SQL Database using this API.
*/

DEPRECATED_BOOLEAN_PREF_IMPL(databasesEnabled, setDatabasesEnabled, true)

/*!
\property OxideQWebPreferences::appCacheEnabled

Whether the offline application cache is enabled. The default is false.
*/

BOOLEAN_PREF_IMPL(appCacheEnabled, setAppCacheEnabled, APP_CACHE_ENABLED)

/*!
\property OxideQWebPreferences::tabsToLinks

Whether HTML \e{anchor} elements are keyboard focusable by pressing the \e{TAB}
key. The default is true.
*/

BOOLEAN_PREF_IMPL(tabsToLinks, setTabsToLinks, TABS_TO_LINKS)

/*!
\property OxideQWebPreferences::caretBrowsingEnabled

Whether to enable caret browsing. The default is false.

When caret browsing is enabled, the content of a web page can be navigated using
the keyboard.
*/

BOOLEAN_PREF_IMPL(caretBrowsingEnabled, setCaretBrowsingEnabled, CARET_BROWSING_ENABLED)

/*!
\property OxideQWebPreferences::touchEnabled
\deprecated

Always true. It is not possible to disable touch events using this API.
*/

DEPRECATED_BOOLEAN_PREF_IMPL(touchEnabled, setTouchEnabled, true)
