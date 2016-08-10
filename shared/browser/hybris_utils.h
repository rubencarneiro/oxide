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

#include "shared/common/oxide_shared_export.h"

namespace oxide {

// Threadsafe class for retreiving device information on devices with libhybris
class OXIDE_SHARED_EXPORT HybrisUtils {
 public:

  struct DeviceProperties {
    std::string product; // ro.product.name
    std::string device; // ro.product.device
    std::string board; // ro.product.board
    std::string brand; // ro.product.brand
    std::string model; // ro.product.model
    std::string os_version; // Parsed version of ro.build.version.release
  };

  virtual ~HybrisUtils();
  static HybrisUtils* GetInstance();

  virtual bool HasDeviceProperties() = 0;

  virtual const DeviceProperties& GetDeviceProperties() = 0;

  virtual bool IsUsingAndroidEGL() = 0;

#if defined(ENABLE_HYBRIS_CAMERA)
  virtual bool IsCameraCompatAvailable() = 0;
#endif

  // Provide a fake HybrisUtils for testing. Note that this is *NOT*
  // threadsafe
  static void OverrideForTesting(HybrisUtils* fake);

 protected:
  HybrisUtils();

 private:
  DISALLOW_COPY_AND_ASSIGN(HybrisUtils);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_HYBRIS_UTILS_H_
