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

#ifndef _OXIDE_SHARED_BROWSER_GEOLOCATION_DELEGATE_H_
#define _OXIDE_SHARED_BROWSER_GEOLOCATION_DELEGATE_H_

#include "base/macros.h"
#include "content/public/browser/geolocation_delegate.h"

namespace oxide {

class GeolocationDelegate : public content::GeolocationDelegate {
 public:
  GeolocationDelegate() = default;

 private:
  // content::GeolocationDelegate implementation
  bool UseNetworkLocationProviders() override;
  scoped_refptr<content::AccessTokenStore> CreateAccessTokenStore() override;
  std::unique_ptr<content::LocationProvider>
  OverrideSystemLocationProvider() override;

  DISALLOW_COPY_AND_ASSIGN(GeolocationDelegate);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_GEOLOCATION_DELEGATE_H_
