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

// This is the main browser process context. It provides access to
// state that is shared between views
class GlobalSettings FINAL {
 public:
  static GlobalSettings* GetInstance();

  OXIDE_EXPORT static std::string GetProduct();
  OXIDE_EXPORT static void SetProduct(const std::string& product);

  OXIDE_EXPORT static std::string GetUserAgent();
  OXIDE_EXPORT static void SetUserAgent(const std::string& user_agent);

  OXIDE_EXPORT static std::string GetDataPath();
  OXIDE_EXPORT static bool SetDataPath(const std::string& data_path);

  OXIDE_EXPORT static std::string GetCachePath();
  OXIDE_EXPORT static bool SetCachePath(const std::string& cache_path);

  OXIDE_EXPORT static std::string GetAcceptLangs();
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
