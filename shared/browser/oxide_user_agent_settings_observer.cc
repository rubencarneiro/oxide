// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include "oxide_user_agent_settings_observer.h"

#include "oxide_user_agent_settings.h"

namespace oxide {

UserAgentSettingsObserver::UserAgentSettingsObserver()
    : settings_(nullptr) {}

UserAgentSettingsObserver::UserAgentSettingsObserver(
    UserAgentSettings* settings)
    : settings_(settings) {
  if (settings) {
    settings->AddObserver(this);
  }
}

void UserAgentSettingsObserver::Observe(UserAgentSettings* settings) {
  if (settings == settings_) {
    return;
  }
  if (settings_) {
    settings_->RemoveObserver(this);
  }
  settings_ = settings;
  if (settings_) {
    settings_->AddObserver(this);
  }
}

UserAgentSettingsObserver::~UserAgentSettingsObserver() {
  if (settings_) {
    settings_->RemoveObserver(this);
  }
}

} // namespace oxide
