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

#ifndef _OXIDE_SHARED_BROWSER_WEB_PREFERENCES_H_
#define _OXIDE_SHARED_BROWSER_WEB_PREFERENCES_H_

#include <string>

#include "base/macros.h"
#include "base/observer_list.h"
#include "base/strings/string16.h"

#include "shared/common/oxide_shared_export.h"

namespace content {
class WebPreferences;
}

namespace oxide {

class WebPreferencesObserver;

class OXIDE_SHARED_EXPORT WebPreferences {
 public:
  WebPreferences();

  enum Attr {
    ATTR_REMOTE_FONTS_ENABLED,

    ATTR_JAVASCRIPT_ENABLED,
    ATTR_ALLOW_SCRIPTS_TO_CLOSE_WINDOWS,
    ATTR_JAVASCRIPT_CAN_ACCESS_CLIPBOARD,

    ATTR_HYPERLINK_AUDITING_ENABLED, // enables anchor ping
    ATTR_ALLOW_UNIVERSAL_ACCESS_FROM_FILE_URLS,
    ATTR_ALLOW_FILE_ACCESS_FROM_FILE_URLS,
    ATTR_CAN_DISPLAY_INSECURE_CONTENT, // Passive mixed-content blocking
    ATTR_CAN_RUN_INSECURE_CONTENT, // Active mixed-content blocking
    ATTR_PASSWORD_ECHO_ENABLED,

    ATTR_LOADS_IMAGES_AUTOMATICALLY,

    ATTR_TEXT_AREAS_ARE_RESIZABLE,

    ATTR_LOCAL_STORAGE_ENABLED,
    ATTR_APP_CACHE_ENABLED,

    ATTR_TABS_TO_LINKS, // whether pressing |TAB\ focuses links
    ATTR_CARET_BROWSING_ENABLED, // Not used anywhere in Chrome. Consider removing

    ATTR_LAST
  };

  std::string StandardFontFamily() const;
  void SetStandardFontFamily(const std::string& font);

  std::string FixedFontFamily() const;
  void SetFixedFontFamily(const std::string& font);

  std::string SerifFontFamily() const;
  void SetSerifFontFamily(const std::string& font);

  std::string SansSerifFontFamily() const;
  void SetSansSerifFontFamily(const std::string& font);

  std::string default_encoding() const {
    return default_encoding_;
  }
  void SetDefaultEncoding(const std::string& encoding);

  unsigned default_font_size() const {
    return default_font_size_;
  }
  void SetDefaultFontSize(unsigned size);

  unsigned default_fixed_font_size() const {
    return default_fixed_font_size_;
  }
  void SetDefaultFixedFontSize(unsigned size);

  unsigned minimum_font_size() const {
    return minimum_font_size_;
  }
  void SetMinimumFontSize(unsigned size);

  bool TestAttribute(Attr attr) const;
  void SetAttribute(Attr attr, bool val);

  void ApplyToWebkitPrefs(content::WebPreferences* prefs);

  virtual void Destroy();
  virtual WebPreferences* Clone() const;

  static WebPreferences* GetFallback();

 protected:
  virtual ~WebPreferences();

  void CopyFrom(const WebPreferences* other);

 private:
  friend class WebPreferencesObserver;

  void NotifyObserversOfChange();

  void AddObserver(WebPreferencesObserver* observer);
  void RemoveObserver(WebPreferencesObserver* observer);

  base::string16 standard_font_family_;
  base::string16 fixed_font_family_;
  base::string16 serif_font_family_;
  base::string16 sans_serif_font_family_;
  std::string default_encoding_;
  unsigned default_font_size_;
  unsigned default_fixed_font_size_;
  unsigned minimum_font_size_;

  bool attributes_[ATTR_LAST];

  base::ObserverList<WebPreferencesObserver> observers_;

  DISALLOW_COPY_AND_ASSIGN(WebPreferences);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_PREFERENCES_H_
