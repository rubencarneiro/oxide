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

#ifndef _OXIDE_PUBLIC_BROWSER_GLOBAL_SETTINGS_H_
#define _OXIDE_PUBLIC_BROWSER_GLOBAL_SETTINGS_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/synchronization/lock.h"

#include "oxide/common/oxide_export.h"

template <typename Type> struct DefaultSingletonTraits;

namespace oxide {

// This provides a way to define and access global settings that are
// shared between views
class GlobalSettings FINAL {
 public:
  static GlobalSettings* GetInstance();

  // Return the product name string. By default, this is "Chrome/<version>"
  OXIDE_EXPORT static std::string GetProduct();
  // Set the product name
  OXIDE_EXPORT static void SetProduct(const std::string& product);

  // Return the user agent string. By default, this is created by
  // Webkit, using the product name
  OXIDE_EXPORT static std::string GetUserAgent();
  // Set the user agent string for all views
  OXIDE_EXPORT static void SetUserAgent(const std::string& user_agent);

  // Return the file path for storing data
  OXIDE_EXPORT static std::string GetDataPath();
  // Set the file path for storing data. This cannot be done after
  // the main browser components have started. By default, this is
  // empty (so all views will run in incognito mode)
  OXIDE_EXPORT static bool SetDataPath(const std::string& data_path);

  // Return the file path for storing cache data
  OXIDE_EXPORT static std::string GetCachePath();
  // Set the file path for storing cache data
  OXIDE_EXPORT static bool SetCachePath(const std::string& cache_path);

  // Return the string used to generate the HTTP Accept-Language header
  // (eg, "en-us,en")
  OXIDE_EXPORT static std::string GetAcceptLangs();
  // Set the string used to generate the HTTP Accept-Language header
  OXIDE_EXPORT static void SetAcceptLangs(const std::string& accept_langs);

 private:
  friend struct DefaultSingletonTraits<GlobalSettings>;

  GlobalSettings() {}

  static void UpdateUserAgentWithWebKit();

  std::string product_;
  std::string user_agent_;
  std::string data_path_;
  std::string cache_path_;
  std::string accept_langs_;

  base::Lock lock_;

  DISALLOW_COPY_AND_ASSIGN(GlobalSettings);
};

} // namespace oxide

#endif // _OXIDE_SHARED_PUBLIC_BROWSER_GLOBAL_SETTINGS_H_
