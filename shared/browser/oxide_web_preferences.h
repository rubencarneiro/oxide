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

#include <set>
#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/strings/string16.h"

class WebPreferences;

namespace oxide {

class WebView;

class WebPreferences {
 public:
  WebPreferences();
  virtual ~WebPreferences();

  enum Attr {
    ATTR_REMOTE_FONTS_ENABLED,

    ATTR_JAVASCRIPT_ENABLED,
    ATTR_WEB_SECURITY_ENABLED,
    ATTR_POPUP_BLOCKER_ENABLED,
    ATTR_ALLOW_SCRIPTS_TO_CLOSE_WINDOWS,
    ATTR_JAVASCRIPT_CAN_ACCESS_CLIPBOARD,

    ATTR_HYPERLINK_AUDITING_ENABLED, // enables anchor ping
    ATTR_ALLOW_UNIVERSAL_ACCESS_FROM_FILE_URLS,
    ATTR_ALLOW_FILE_ACCESS_FROM_FILE_URLS,
    ATTR_CAN_DISPLAY_INSECURE_CONTENT, // Passive mixed-content blocking
    ATTR_CAN_RUN_INSECURE_CONTENT, // Active mixed-content blocking
    ATTR_PASSWORD_ECHO_ENABLED,

    ATTR_LOADS_IMAGES_AUTOMATICALLY,
    ATTR_SHRINKS_STANDALONE_IMAGES_TO_FIT,

    ATTR_TEXT_AREAS_ARE_RESIZABLE,

    ATTR_LOCAL_STORAGE_ENABLED,
    ATTR_DATABASES_ENABLED,
    ATTR_APP_CACHE_ENABLED,
    ATTR_FULLSCREEN_ENABLED,

    ATTR_TABS_TO_LINKS, // whether pressing |TAB\ focuses links
    ATTR_CARET_BROWSING_ENABLED,

    ATTR_SMOOTH_SCROLLING_ENABLED,
    ATTR_TOUCH_ENABLED,
    ATTR_SUPPORTS_MULTIPLE_WINDOWS,

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

  void AddWebView(WebView* view);
  void RemoveWebView(WebView* view);

  void ApplyToWebkitPrefs(::WebPreferences* prefs);

 private:
  void UpdateViews();

  base::string16 standard_font_family_;
  base::string16 fixed_font_family_;
  base::string16 serif_font_family_;
  base::string16 sans_serif_font_family_;
  std::string default_encoding_;
  unsigned default_font_size_;
  unsigned default_fixed_font_size_;
  unsigned minimum_font_size_;

  bool attributes_[ATTR_LAST];

  std::set<WebView *> views_;

  DISALLOW_COPY_AND_ASSIGN(WebPreferences);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_PREFERENCES_H_
