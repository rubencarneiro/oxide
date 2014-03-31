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

#include "oxide_web_preferences.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "webkit/common/webpreferences.h"

#include "oxide_web_preferences_observer.h"

namespace oxide {

void WebPreferences::NotifyObserversOfChange() {
  FOR_EACH_OBSERVER(WebPreferencesObserver,
                    observers_,
                    WebPreferencesValueChanged());
}

void WebPreferences::AddObserver(WebPreferencesObserver* observer) {
  observers_.AddObserver(observer);
}

void WebPreferences::RemoveObserver(WebPreferencesObserver* observer) {
  observers_.RemoveObserver(observer);
}

WebPreferences::WebPreferences() :
    standard_font_family_(base::UTF8ToUTF16("Times New Roman")),
    fixed_font_family_(base::UTF8ToUTF16("Courier New")),
    serif_font_family_(base::UTF8ToUTF16("Times New Roman")),
    sans_serif_font_family_(base::UTF8ToUTF16("Arial")),
    default_encoding_("ISO-8859-1"),
    default_font_size_(16),
    default_fixed_font_size_(13),
    minimum_font_size_(0) {
  for (unsigned int i = 0; i < ATTR_LAST; ++i) {
    attributes_[i] = false;
  }

  SetAttribute(ATTR_REMOTE_FONTS_ENABLED, true);
  SetAttribute(ATTR_JAVASCRIPT_ENABLED, true);
  SetAttribute(ATTR_POPUP_BLOCKER_ENABLED, true);

  // ATTR_ALLOW_SCRIPTS_TO_CLOSE_WINDOWS
  // ATTR_JAVASCRIPT_CAN_ACCESS_CLIPBOARD

  SetAttribute(ATTR_HYPERLINK_AUDITING_ENABLED, true);

  // ATTR_ALLOW_UNIVERSAL_ACCESS_FROM_FILE_URLS
  // ATTR_ALLOW_FILE_ACCESS_FROM_FILE_URLS
  // ATTR_CAN_DISPLAY_INSECURE_CONTENT
  // ATTR_CAN_RUN_INSECURE_CONTENT
  // ATTR_PASSWORD_ECHO_ENABLED

  SetAttribute(ATTR_LOADS_IMAGES_AUTOMATICALLY, true);
  SetAttribute(ATTR_SHRINKS_STANDALONE_IMAGES_TO_FIT, true);

  SetAttribute(ATTR_TEXT_AREAS_ARE_RESIZABLE, true);

  // ATTR_LOCAL_STORAGE_ENABLED
  // ATTR_DATABASES_ENABLED
  // ATTR_APP_CACHE_ENABLED

  SetAttribute(ATTR_TABS_TO_LINKS, true);

  // ATTR_CARET_BROWSING_ENABLED

  SetAttribute(ATTR_TOUCH_ENABLED, true);

  // ATTR_SUPPORTS_MULTIPLE_WINDOWS
}

WebPreferences::~WebPreferences() {
  FOR_EACH_OBSERVER(WebPreferencesObserver,
                    observers_,
                    OnWebPreferencesDestruction());
}

std::string WebPreferences::StandardFontFamily() const {
  return base::UTF16ToUTF8(standard_font_family_);
}

void WebPreferences::SetStandardFontFamily(const std::string& font) {
  base::string16 value = base::UTF8ToUTF16(font);
  if (value == standard_font_family_) {
    return;
  }
  standard_font_family_ = value;
  NotifyObserversOfChange();
}

std::string WebPreferences::FixedFontFamily() const {
  return base::UTF16ToUTF8(fixed_font_family_);
}

void WebPreferences::SetFixedFontFamily(const std::string& font) {
  base::string16 value = base::UTF8ToUTF16(font);
  if (value == fixed_font_family_) {
    return;
  }
  fixed_font_family_ = value;
  NotifyObserversOfChange();
}

std::string WebPreferences::SerifFontFamily() const {
  return base::UTF16ToUTF8(serif_font_family_);
}

void WebPreferences::SetSerifFontFamily(const std::string& font) {
  base::string16 value = base::UTF8ToUTF16(font);
  if (value == serif_font_family_) {
    return;
  }
  serif_font_family_ = value;
  NotifyObserversOfChange();
}

std::string WebPreferences::SansSerifFontFamily() const {
  return base::UTF16ToUTF8(sans_serif_font_family_);
}

void WebPreferences::SetSansSerifFontFamily(const std::string& font) {
  base::string16 value = base::UTF8ToUTF16(font);
  if (value == sans_serif_font_family_) {
    return;
  }
  sans_serif_font_family_ = value;
  NotifyObserversOfChange();
}

void WebPreferences::SetDefaultEncoding(const std::string& encoding) {
  if (encoding == default_encoding_) {
    return;
  }
  default_encoding_ = encoding;
  NotifyObserversOfChange();
}

void WebPreferences::SetDefaultFontSize(unsigned size) {
  if (size == default_font_size_) {
    return;
  }
  default_font_size_ = size;
  NotifyObserversOfChange();
}

void WebPreferences::SetDefaultFixedFontSize(unsigned size) {
  if (size == default_fixed_font_size_) {
    return;
  }
  default_fixed_font_size_ = size;
  NotifyObserversOfChange();
}

void WebPreferences::SetMinimumFontSize(unsigned size) {
  if (size == minimum_font_size_) {
    return;
  }
  minimum_font_size_ = size;
  NotifyObserversOfChange();
}

bool WebPreferences::TestAttribute(Attr attr) const {
  CHECK(attr < ATTR_LAST && attr >= 0);
  return attributes_[attr];
}

void WebPreferences::SetAttribute(Attr attr, bool val) {
  CHECK(attr < ATTR_LAST && attr >= 0);
  if (val == attributes_[attr]) {
    return;
  }

  if (attr == ATTR_SUPPORTS_MULTIPLE_WINDOWS && val) {
    LOG(WARNING) <<
        "Oxide currently doesn't support window.open(). "
        "See https://launchpad.net/bugs/1240749";
    return;
  }

  attributes_[attr] = val;
  NotifyObserversOfChange();
}

void WebPreferences::ApplyToWebkitPrefs(::WebPreferences* prefs) {
  prefs->standard_font_family_map[webkit_glue::kCommonScript] =
      standard_font_family_;
  prefs->fixed_font_family_map[webkit_glue::kCommonScript] =
      fixed_font_family_;
  prefs->serif_font_family_map[webkit_glue::kCommonScript] =
      serif_font_family_;
  prefs->sans_serif_font_family_map[webkit_glue::kCommonScript] =
      sans_serif_font_family_;

  prefs->default_encoding = default_encoding_;

  prefs->default_font_size = default_font_size_;
  prefs->default_fixed_font_size = default_fixed_font_size_;
  prefs->minimum_font_size = minimum_font_size_;

  prefs->remote_fonts_enabled = attributes_[ATTR_REMOTE_FONTS_ENABLED];

  prefs->javascript_enabled = attributes_[ATTR_JAVASCRIPT_ENABLED];
  prefs->javascript_can_open_windows_automatically =
      !attributes_[ATTR_POPUP_BLOCKER_ENABLED];
  prefs->allow_scripts_to_close_windows =
      attributes_[ATTR_ALLOW_SCRIPTS_TO_CLOSE_WINDOWS];
  prefs->javascript_can_access_clipboard =
      attributes_[ATTR_JAVASCRIPT_CAN_ACCESS_CLIPBOARD];

  prefs->hyperlink_auditing_enabled =
      attributes_[ATTR_HYPERLINK_AUDITING_ENABLED];
  prefs->allow_universal_access_from_file_urls =
      attributes_[ATTR_ALLOW_UNIVERSAL_ACCESS_FROM_FILE_URLS];
  prefs->allow_file_access_from_file_urls =
      attributes_[ATTR_ALLOW_FILE_ACCESS_FROM_FILE_URLS];
  prefs->allow_displaying_insecure_content =
      attributes_[ATTR_CAN_DISPLAY_INSECURE_CONTENT];
  prefs->allow_running_insecure_content =
      attributes_[ATTR_CAN_RUN_INSECURE_CONTENT];
  prefs->password_echo_enabled = attributes_[ATTR_PASSWORD_ECHO_ENABLED];

  prefs->loads_images_automatically =
      attributes_[ATTR_LOADS_IMAGES_AUTOMATICALLY];
  prefs->shrinks_standalone_images_to_fit =
      attributes_[ATTR_SHRINKS_STANDALONE_IMAGES_TO_FIT];

  prefs->text_areas_are_resizable = attributes_[ATTR_TEXT_AREAS_ARE_RESIZABLE];

  prefs->local_storage_enabled = attributes_[ATTR_LOCAL_STORAGE_ENABLED];
  prefs->databases_enabled = attributes_[ATTR_DATABASES_ENABLED];
  prefs->application_cache_enabled = attributes_[ATTR_APP_CACHE_ENABLED];

  prefs->tabs_to_links = attributes_[ATTR_TABS_TO_LINKS];
  prefs->caret_browsing_enabled = attributes_[ATTR_CARET_BROWSING_ENABLED];

  prefs->touch_enabled = attributes_[ATTR_TOUCH_ENABLED];

  prefs->supports_multiple_windows = attributes_[ATTR_SUPPORTS_MULTIPLE_WINDOWS];
}

} // namespace oxide
