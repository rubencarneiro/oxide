// vim:expandtab:shiftwidth=2:tabstop=2:
// copyright (c) 2013 canonical ltd.

// this library is free software; you can redistribute it and/or
// modify it under the terms of the gnu lesser general public
// license as published by the free software foundation; either
// version 2.1 of the license, or (at your option) any later version.

// this library is distributed in the hope that it will be useful,
// but without any warranty; without even the implied warranty of
// merchantability or fitness for a particular purpose.  see the gnu
// lesser general public license for more details.

// you should have received a copy of the gnu lesser general public
// license along with this library; if not, write to the free software
// foundation, inc., 51 franklin street, fifth floor, boston, ma  02110-1301  usa

#include "web_preferences.h"

#include <map>

#include <QFont>
#include <QString>

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/common/web_preferences.h"

#include "shared/browser/web_preferences.h"

namespace oxide {
namespace qt {

namespace {

base::LazyInstance<std::map<oxide::WebPreferences*,
                            WebPreferences*>> g_prefs_map =
    LAZY_INSTANCE_INITIALIZER;

content::ScriptFontFamilyMap MakeDefaultScriptFontFamilyMap(const QFont& font) {
  return content::ScriptFontFamilyMap(
      {{ content::kCommonScript,
         base::UTF8ToUTF16(font.defaultFamily().toStdString()) }});
}

void AddMapping(oxide::WebPreferences* from, WebPreferences* to) {
  bool result = g_prefs_map.Get().insert(std::make_pair(from, to)).second;
  DCHECK(result);
}

void RemoveMapping(oxide::WebPreferences* prefs) {
  size_t erased = g_prefs_map.Get().erase(prefs);
  DCHECK_GT(erased, 0U);
}

}

WebPreferences::WebPreferences(OxideQWebPreferences* api_handle)
    : prefs_(new oxide::WebPreferences()),
      api_handle_(api_handle) {
  AddMapping(prefs_.get(), this);

  QFont font;

  font.setStyleHint(QFont::Serif);
  prefs_->set_standard_font_family_map(MakeDefaultScriptFontFamilyMap(font));
  prefs_->set_serif_font_family_map(MakeDefaultScriptFontFamilyMap(font));

  font.setStyleHint(QFont::SansSerif);
  prefs_->set_sans_serif_font_family_map(MakeDefaultScriptFontFamilyMap(font));

  font.setStyleHint(QFont::Monospace);
  prefs_->set_fixed_font_family_map(MakeDefaultScriptFontFamilyMap(font));
}

WebPreferences::~WebPreferences() {
  RemoveMapping(prefs_.get());
}

// static
WebPreferences* WebPreferences::FromPrefs(oxide::WebPreferences* prefs) {
  auto it = g_prefs_map.Get().find(prefs);
  return it == g_prefs_map.Get().end() ? nullptr : it->second;
}

oxide::WebPreferences* WebPreferences::GetPrefs() const {
  return prefs_.get();
}

void WebPreferences::AdoptPrefs(oxide::WebPreferences* prefs) {
  RemoveMapping(prefs_.get());
  prefs_ = prefs;
  AddMapping(prefs_.get(), this);
}

} // namespace qt
} // namespace oxide
