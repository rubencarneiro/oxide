// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include "oxide_browser_platform_integration_observer.h"

namespace oxide {

BrowserPlatformIntegrationObserver::BrowserPlatformIntegrationObserver()
    : platform_(NULL) {}

BrowserPlatformIntegrationObserver::BrowserPlatformIntegrationObserver(
    BrowserPlatformIntegration* platform) : platform_(platform) {
  if (platform) {
    platform->AddObserver(this);
  }
}

void BrowserPlatformIntegrationObserver::Observe(
    BrowserPlatformIntegration* platform) {
  if (platform_ == platform) {
    return;
  }
  if (platform_) {
    platform_->RemoveObserver(this);
  }
  platform_ = platform;
  if (platform_) {
    platform_->AddObserver(this);
  }
}

BrowserPlatformIntegrationObserver::~BrowserPlatformIntegrationObserver() {
  if (platform_) {
    platform_->RemoveObserver(this);
  }
}

} // namespace oxide
