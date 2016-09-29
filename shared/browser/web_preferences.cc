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

#include "web_preferences.h"

#include "base/strings/utf_string_conversions.h"

namespace oxide {

WebPreferences::~WebPreferences() = default;

#define WEB_PREF(type, name, initial_value) \
      name##_(initial_value),
WebPreferences::WebPreferences()
    :
#include "web_preferences_list.h"
#undef WEB_PREF
      dummy_(false) {}

#define WEB_PREF(type, name, initial_value) \
void WebPreferences::set_##name(const type& value) { \
  name##_ = value; \
  callback_list_.Notify(); \
}
#include "web_preferences_list.h"
#undef WEB_PREF
  
void WebPreferences::ApplyToWebkitPrefs(content::WebPreferences* prefs) const {
#define WEB_PREF(type, name, initial_value) \
  prefs->name = name##_;
#include "web_preferences_list.h"
#undef WEB_PREF
}

scoped_refptr<WebPreferences> WebPreferences::Clone() {
  scoped_refptr<WebPreferences> cloned_prefs = new WebPreferences();

#define WEB_PREF(type, name, initial_value) \
  cloned_prefs->name##_ = name##_;
#include "web_preferences_list.h"
#undef WEB_PREF

  return cloned_prefs;
}

std::unique_ptr<WebPreferences::Subscription> WebPreferences::AddChangeCallback(
    const ObserverCallback& callback) {
  return callback_list_.Add(callback);
}

} // namespace oxide
