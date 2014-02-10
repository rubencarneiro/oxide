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

#include "../oxideqwebpreferences_p.h"
#include "../oxideqwebpreferences.h"

#include "qt/core/glue/oxide_qt_web_view_adapter.h"

OxideQWebPreferencesPrivate::OxideQWebPreferencesPrivate() :
    in_destructor_(false) {}

OxideQWebPreferencesPrivate::~OxideQWebPreferencesPrivate() {
  in_destructor_ = true;

  for (std::set<oxide::qt::WebViewAdapter *>::const_iterator it = views_.begin();
       it != views_.end(); ++it) {
    oxide::qt::WebViewAdapter* view = *it;
    view->NotifyWebPreferencesDestroyed();
  }
}

void OxideQWebPreferencesPrivate::AddWebView(oxide::qt::WebViewAdapter* view) {
  Q_ASSERT(!in_destructor_);
  views_.insert(view);
}

void OxideQWebPreferencesPrivate::RemoveWebView(
    oxide::qt::WebViewAdapter* view) {
  if (in_destructor_) {
    return;
  }

  views_.erase(view);
}

// static
OxideQWebPreferencesPrivate* OxideQWebPreferencesPrivate::get(OxideQWebPreferences* q) {
  return q->d_func();
}
