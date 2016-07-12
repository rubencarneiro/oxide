// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#include "oxide_geolocation_delegate.h"

#include "content/public/browser/location_provider.h"

#include "oxide_browser_platform_integration.h"

namespace oxide {

bool GeolocationDelegate::UseNetworkLocationProviders() {
  return false;
}

content::AccessTokenStore* GeolocationDelegate::CreateAccessTokenStore() {
  return nullptr;
}

std::unique_ptr<content::LocationProvider>
GeolocationDelegate::OverrideSystemLocationProvider() {
  return BrowserPlatformIntegration::GetInstance()->CreateLocationProvider();
}

} // namespace oxide
