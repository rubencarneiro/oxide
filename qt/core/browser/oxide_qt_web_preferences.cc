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

#include "oxide_qt_web_preferences.h"

#include <QFont>
#include <QString>

#include "base/logging.h"

namespace oxide {
namespace qt {

WebPreferences::WebPreferences(OxideQWebPreferences* api_handle) :
    api_handle_(api_handle) {
  QFont font;

  font.setStyleHint(QFont::Serif);
  SetStandardFontFamily(font.defaultFamily().toStdString());
  SetSerifFontFamily(font.defaultFamily().toStdString());

  font.setStyleHint(QFont::SansSerif);
  SetSansSerifFontFamily(font.defaultFamily().toStdString());

  font.setStyleHint(QFont::Monospace);
  SetFixedFontFamily(font.defaultFamily().toStdString());
}

WebPreferences::~WebPreferences() {
  CHECK(!api_handle_);
}

} // namespace qt
} // namespace oxide
