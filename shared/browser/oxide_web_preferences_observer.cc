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

#include "oxide_web_preferences_observer.h"

#include "base/logging.h"

#include "oxide_web_preferences.h"

namespace oxide {

void WebPreferencesObserver::OnWebPreferencesDestruction() {
  DCHECK(web_preferences_);
  web_preferences_ = NULL;
  WebPreferencesDestroyed();
}

WebPreferencesObserver::WebPreferencesObserver() :
    web_preferences_(NULL) {}

void WebPreferencesObserver::Observe(WebPreferences* preferences) {
  if (web_preferences_ == preferences) {
    return;
  }
  if (web_preferences_) {
    web_preferences_->RemoveObserver(this);
  }
  web_preferences_ = preferences;
  if (web_preferences_) {
    web_preferences_->AddObserver(this);
  }
}

WebPreferencesObserver::~WebPreferencesObserver() {
  if (web_preferences_) {
    web_preferences_->RemoveObserver(this);
  }
}

} // namespace oxide
