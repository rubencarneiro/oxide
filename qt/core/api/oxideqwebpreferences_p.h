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

#ifndef _OXIDE_QT_CORE_API_WEB_PREFERENCES_P_H_
#define _OXIDE_QT_CORE_API_WEB_PREFERENCES_P_H_

#include <QtGlobal>

#include "base/memory/scoped_ptr.h"

class OxideQWebPreferences;

QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE

namespace oxide {
namespace qt {
class WebPreferences;
}
}

class OxideQWebPreferencesPrivate final {
 public:
  ~OxideQWebPreferencesPrivate();

  oxide::qt::WebPreferences* preferences() const { return preferences_.get(); }

  static OxideQWebPreferencesPrivate* get(OxideQWebPreferences* q);

  static OxideQWebPreferences* Adopt(oxide::qt::WebPreferences* preferences,
                                     QObject* parent = NULL);

 private:
  friend class OxideQWebPreferences;
  OxideQWebPreferencesPrivate(OxideQWebPreferences* q);
  OxideQWebPreferencesPrivate(oxide::qt::WebPreferences* preferences);

  scoped_ptr<oxide::qt::WebPreferences> preferences_;
};

#endif // _OXIDE_QT_CORE_API_WEB_PREFERENCES_P_H_
