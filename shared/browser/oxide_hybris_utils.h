// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015-2016 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_HYBRIS_UTILS_H_
#define _OXIDE_SHARED_BROWSER_HYBRIS_UTILS_H_

#include <string>

#include "base/macros.h"

namespace oxide {

class HybrisUtils {
 public:

  struct DeviceProperties {
    std::string product; // ro.product.name
    std::string device; // ro.product.device
    std::string board; // ro.product.board
    std::string brand; // ro.product.brand
    std::string model; // ro.product.model
    std::string os_version; // Parsed version of ro.build.version.release
  };

  static bool HasDeviceProperties();

  static const DeviceProperties& GetDeviceProperties();

  static bool IsUsingAndroidEGL();

#if defined(ENABLE_HYBRIS_CAMERA)
  static bool IsCameraCompatAvailable();
#endif

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(HybrisUtils);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_HYBRIS_UTILS_H_
