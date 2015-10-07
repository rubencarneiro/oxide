// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_ANDROID_PROPERTIES_H_
#define _OXIDE_SHARED_BROWSER_ANDROID_PROPERTIES_H_

#include <string>

#include "base/macros.h"

namespace base {
template <typename T> struct DefaultSingletonTraits;
}

namespace oxide {

// Provides some values from /dev/socket/system_properties if we're
// running on an Android-based system (ie, Ubuntu Phone). Can be
// used on any thread
class AndroidProperties {
 public:
  ~AndroidProperties();

  // Return the singleton
  static AndroidProperties* GetInstance();

  // Whether the system properties are available. If they're not, then
  // this is probably not an Android-based system
  bool Available() const;

  // These methods DCHECK that system properties are available

  // Return ro.product.name
  std::string GetProduct() const;

  // Return ro.product.device
  std::string GetDevice() const;

  // Return ro.product.board
  std::string GetBoard() const;

  // Return ro.product.brand
  std::string GetBrand() const;

  // Return ro.product.model
  std::string GetModel() const;

  // Returns the parsed value of ro.build.version.release
  std::string GetOSVersion() const;

 private:
  friend struct base::DefaultSingletonTraits<AndroidProperties>;

  AndroidProperties();

  bool available_;

  std::string product_;
  std::string device_;
  std::string board_;
  std::string brand_;
  std::string model_;
  std::string os_version_;

  DISALLOW_COPY_AND_ASSIGN(AndroidProperties);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_ANDROID_PROPERTIES_H_
