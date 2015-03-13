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

#ifndef _OXIDE_QT_CORE_BROWSER_WEB_PREFERENCES_H_
#define _OXIDE_QT_CORE_BROWSER_WEB_PREFERENCES_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"

#include "shared/browser/oxide_web_preferences.h"

class OxideQWebPreferences;

namespace oxide {
namespace qt {

class WebPreferences final : public oxide::WebPreferences {
 public:
  WebPreferences(OxideQWebPreferences* api_handle = nullptr);
  ~WebPreferences();

  OxideQWebPreferences* api_handle() const { return api_handle_; }

  // oxide::WebPreferences implementation
  void Destroy() final;
  oxide::WebPreferences* Clone() const final;

 private:
  OxideQWebPreferences* api_handle_;

  DISALLOW_COPY_AND_ASSIGN(WebPreferences);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_WEB_PREFERENCES_H_
