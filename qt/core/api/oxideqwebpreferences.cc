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

#include "base/strings/utf_string_conversions.h"
#include "content/public/common/web_preferences.h"

#include "shared/browser/web_preferences.h"

#include "oxideqglobal_p.h"

OxideQWebPreferencesPrivate::OxideQWebPreferencesPrivate(
    OxideQWebPreferences* q)
    : preferences_(q) {}

OxideQWebPreferencesPrivate::~OxideQWebPreferencesPrivate() = default;

// static
OxideQWebPreferencesPrivate* OxideQWebPreferencesPrivate::get(
    OxideQWebPreferences* q) {
  return q->d_func();
}

oxide::WebPreferences* OxideQWebPreferencesPrivate::GetPrefs() const {
  return preferences_.GetPrefs();
}

void OxideQWebPreferencesPrivate::AdoptPrefs(oxide::WebPreferences* prefs) {
  preferences_.AdoptPrefs(prefs);
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

OxideQWebPreferences::~OxideQWebPreferences() = default;

/*!
Construct a new preferences instance and make it a child of \a{parent}.
*/

OxideQWebPreferences::OxideQWebPreferences(QObject* parent)
    : QObject(parent),
      d_ptr(new OxideQWebPreferencesPrivate(this)) {}

#define FONT_FAMILY_PREF_IMPL(getter, setter, map_pref) \
QString OxideQWebPreferences::getter() const { \
  Q_D(const OxideQWebPreferences); \
  const content::ScriptFontFamilyMap& map = d->GetPrefs()->map_pref##_map(); \
  if (map.find(content::kCommonScript) == map.end()) { \
    return QString(); \
  } \
\
  return QString::fromStdString( \
      base::UTF16ToUTF8(map.at(content::kCommonScript))); \
} \
\
void OxideQWebPreferences::setter(const QString& font) { \
  Q_D(OxideQWebPreferences); \
\
  if (font == getter()) { \
    return; \
  } \
\
  content::ScriptFontFamilyMap map = d->GetPrefs()->map_pref##_map(); \
  if (font.isEmpty()) { \
    map.erase(content::kCommonScript); \
  } else { \
    map[content::kCommonScript] = base::UTF8ToUTF16(font.toStdString()); \
  } \
\
  d->GetPrefs()->set_##map_pref##_map(map); \
  Q_EMIT getter##Changed(); \
}

#define STRING_PREF_IMPL(getter, setter, pref) \
QString OxideQWebPreferences::getter() const { \
  Q_D(const OxideQWebPreferences); \
  return QString::fromStdString(d->GetPrefs()->pref()); \
} \
\
void OxideQWebPreferences::setter(const QString& value) { \
  Q_D(OxideQWebPreferences); \
\
  if (value == getter()) { \
    return; \
  } \
\
  d->GetPrefs()->set_##pref(value.toStdString()); \
  Q_EMIT getter##Changed(); \
}

#define BOOLEAN_PREF_IMPL(getter, setter, pref) \
bool OxideQWebPreferences::getter() const { \
  Q_D(const OxideQWebPreferences); \
  return d->GetPrefs()->pref(); \
} \
\
void OxideQWebPreferences::setter(bool value) { \
  Q_D(OxideQWebPreferences); \
\
  if (value == getter()) { \
    return; \
  } \
\
  d->GetPrefs()->set_##pref(value); \
  Q_EMIT getter##Changed(); \
}

#define UNSIGNED_PREF_IMPL(getter, setter, pref) \
unsigned OxideQWebPreferences::getter() const { \
  Q_D(const OxideQWebPreferences); \
  return d->GetPrefs()->pref(); \
} \
\
void OxideQWebPreferences::setter(unsigned value) { \
  Q_D(OxideQWebPreferences); \
\
  if (value == getter()) { \
    return; \
  } \
\
  d->GetPrefs()->set_##pref(value); \
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
\property OxideQWebPreferences::standardFontFamily

The default standard font family for characters from the Unicode \e{Common}
script.

The default value is dependent on the system's font configuration.
*/

FONT_FAMILY_PREF_IMPL(standardFontFamily,
                      setStandardFontFamily,
                      standard_font_family)

/*!
\property OxideQWebPreferences::fixedFontFamily

The default fixed font family for characters from the Unicode \e{Common} script.

The default value is dependent on the system's font configuration.
*/

FONT_FAMILY_PREF_IMPL(fixedFontFamily, setFixedFontFamily, fixed_font_family)

/*!
\property OxideQWebPreferences::serifFontFamily

The default Serif font family for characters from the Unicode \e{Common} script.

The default value is dependent on the system's font configuration.
*/

FONT_FAMILY_PREF_IMPL(serifFontFamily, setSerifFontFamily, serif_font_family)

/*!
\property OxideQWebPreferences::sanSerifFontFamily

The default Sans Serif font family for characters from the Unicode \e{Common}
script.

The default value is dependent on the system's font configuration.
*/

FONT_FAMILY_PREF_IMPL(sansSerifFontFamily,
                      setSansSerifFontFamily,
                      sans_serif_font_family)

/*!
\property OxideQWebPreferences::defaultEncoding

The default character set, used for pages that don't define a character set.
*/

STRING_PREF_IMPL(defaultEncoding, setDefaultEncoding, default_encoding)

/*!
\property OxideQWebPreferences::defaultFontSize

The default font size in points.
*/

UNSIGNED_PREF_IMPL(defaultFontSize, setDefaultFontSize, default_font_size)

/*!
\property OxideQWebPreferences::defaultFixedFontSize

The default fixed font size in points.
*/

UNSIGNED_PREF_IMPL(defaultFixedFontSize,
                   setDefaultFixedFontSize,
                   default_fixed_font_size)

/*!
\property OxideQWebPreferences::minimumFontSize

The minimum font size in points.
*/

UNSIGNED_PREF_IMPL(minimumFontSize, setMinimumFontSize, minimum_font_size)

/*!
\property OxideQWebPreferences::remoteFontsEnabled

Whether support for remote web fonts is enabled. The default is true.
*/

BOOLEAN_PREF_IMPL(remoteFontsEnabled,
                  setRemoteFontsEnabled,
                  remote_fonts_enabled)

/*!
\property OxideQWebPreferences::javascriptEnabled

Whether JavaScript is enabled. The default is true.
*/

BOOLEAN_PREF_IMPL(javascriptEnabled, setJavascriptEnabled, javascript_enabled)

/*!
\property OxideQWebPreferences::allowScriptsToCloseWindows

Whether web content can request to close a window via \e{window.close}. The
default is false.

\note Web content can always request to close windows that were opened by it via
\e{window.open()}, regardless of this preference.
*/

BOOLEAN_PREF_IMPL(allowScriptsToCloseWindows,
                  setAllowScriptsToCloseWindows,
                  allow_scripts_to_close_windows)

/*!
\property OxideQWebPreferences::javascriptCanAccessClipboard

Whether javascript can use the various clipboard related editing commands via
\e{document.execCommand()}. The default is false.
*/

BOOLEAN_PREF_IMPL(javascriptCanAccessClipboard,
                  setJavascriptCanAccessClipboard,
                  javascript_can_access_clipboard)

/*!
\property OxideQWebPreferences::hyperlinkAuditingEnabled

Whether to notify the URLs specified by the \e{ping} attribute when a user
clicks on an \e{anchor} element. The default is true.
*/

BOOLEAN_PREF_IMPL(hyperlinkAuditingEnabled,
                  setHyperlinkAuditingEnabled,
                  hyperlink_auditing_enabled)

/*!
\property OxideQWebPreferences::allowUniversalAccessFromFileUrls

Whether to disable same-origin restrictions for pages loaded via file: URLs. The
default is false.

\note This is a dangerous option and should generally not be used in production
code. Application developers should think carefully before enabling this option
to ensure that they are fully aware of its consequences. This option should
never be enabled in web browsers.
*/

BOOLEAN_PREF_IMPL(allowUniversalAccessFromFileUrls,
                  setAllowUniversalAccessFromFileUrls,
                  allow_universal_access_from_file_urls)

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

BOOLEAN_PREF_IMPL(allowFileAccessFromFileUrls,
                  setAllowFileAccessFromFileUrls,
                  allow_file_access_from_file_urls)

/*!
\property OxideQWebPreferences::canDisplayInsecureContent

Whether to allow the display of passive mixed content. The default is true.

Passive mixed content includes images and videos loaded over an insecure
connection in to a page that was loaded over a secure connection. Passive mixed
content can't access other parts of a web page or change the behaviour of it,
but an attacker could replace the content a user sees or otherwise infer the
user's browsing habits because the connections aren't private.
*/

BOOLEAN_PREF_IMPL(canDisplayInsecureContent,
                  setCanDisplayInsecureContent,
                  allow_displaying_insecure_content)

/*!
\property OxideQWebPreferences::canRunInsecureContent

Whether to allow the execution of active mixed content. The default is false.

Active mixed content includes CSS and scripts loaded over an insecure connection
in to a page that was loaded over a secure connection. Because active mixed
content can change the behaviour of a page and steal sensitive information, it
compromises the security of a page entirely.
*/

BOOLEAN_PREF_IMPL(canRunInsecureContent,
                  setCanRunInsecureContent,
                  allow_running_insecure_content)

/*!
\property OxideQWebPreferences::passwordEchoEnabled

Whether password echo is enabled. The default is false.

When password echo is enabled, actual characters are displayed when a user
inputs text in to an \e{<input type="password">} element.
*/

BOOLEAN_PREF_IMPL(passwordEchoEnabled,
                  setPasswordEchoEnabled,
                  password_echo_enabled)

/*!
\property OxideQWebPreferences::loadsImagesAutomatically

Whether to load images automatically when a page loads. The default is true.

If this is set to false, images aren't loaded when a page loads, and image
elements are replaced by a placeholder.
*/

BOOLEAN_PREF_IMPL(loadsImagesAutomatically,
                  setLoadsImagesAutomatically,
                  loads_images_automatically)

/*!
\property OxideQWebPreferences::shrinksStandaloneImagesToFit
\deprecated

Always true. Because of a bug, this preference has never had any effect.
*/

DEPRECATED_BOOLEAN_PREF_IMPL(shrinksStandaloneImagesToFit,
                             setShrinksStandaloneImagesToFit,
                             true)

/*!
\property OxideQWebPreferences::textAreasAreResizable

Whether \e{<textarea>} elements are resizable. The default is true.
*/

BOOLEAN_PREF_IMPL(textAreasAreResizable,
                  setTextAreasAreResizable,
                  text_areas_are_resizable)

/*!
\property OxideQWebPreferences::localStorageEnabled

Whether the DOM local storage API is enabled. The default is false.
*/

BOOLEAN_PREF_IMPL(localStorageEnabled,
                  setLocalStorageEnabled,
                  local_storage_enabled)

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

BOOLEAN_PREF_IMPL(appCacheEnabled,
                  setAppCacheEnabled,
                  application_cache_enabled)

/*!
\property OxideQWebPreferences::tabsToLinks

Whether HTML \e{anchor} elements are keyboard focusable by pressing the \e{TAB}
key. The default is true.
*/

BOOLEAN_PREF_IMPL(tabsToLinks, setTabsToLinks, tabs_to_links)

/*!
\property OxideQWebPreferences::caretBrowsingEnabled

Whether to enable caret browsing. The default is false.

When caret browsing is enabled, the content of a web page can be navigated using
the keyboard.
*/

BOOLEAN_PREF_IMPL(caretBrowsingEnabled,
                  setCaretBrowsingEnabled,
                  caret_browsing_enabled)

/*!
\property OxideQWebPreferences::touchEnabled
\deprecated

Always true. It is not possible to disable touch events using this API.
*/

DEPRECATED_BOOLEAN_PREF_IMPL(touchEnabled, setTouchEnabled, true)
