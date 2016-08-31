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

#include "power_save_blocker.h"

#include <memory>

#include "base/environment.h"
#include "base/memory/ptr_util.h"
#include "base/nix/xdg_util.h"

#include "power_save_blocker_fdo.h"
#include "power_save_blocker_unity8.h"

namespace oxide {

PowerSaveBlocker::~PowerSaveBlocker() = default;

// static
std::unique_ptr<PowerSaveBlocker> PowerSaveBlocker::Create(
    device::PowerSaveBlocker::PowerSaveBlockerType type,
    device::PowerSaveBlocker::Reason reason,
    const std::string& description) {
  std::unique_ptr<base::Environment> env = base::Environment::Create();

  // This is kind of hacky - there's no way to identify that we're running on
  // Unity 8, so we just assume that we are if QT_QPA_PLATFORM is set to
  // "ubuntumirclient"
  std::string qt_qpa_platform;
  if (env->GetVar("QT_QPA_PLATFORM", &qt_qpa_platform) &&
      qt_qpa_platform == "ubuntumirclient") {
    return base::MakeUnique<PowerSaveBlockerUnity8>();
  }

  base::nix::DesktopEnvironment de =
      base::nix::GetDesktopEnvironment(env.get());
  switch (de) {
    case base::nix::DESKTOP_ENVIRONMENT_GNOME:
    case base::nix::DESKTOP_ENVIRONMENT_KDE4:
    case base::nix::DESKTOP_ENVIRONMENT_KDE5:
    case base::nix::DESKTOP_ENVIRONMENT_UNITY:
    case base::nix::DESKTOP_ENVIRONMENT_XFCE:
      // FIXME: Unity and Gnome don't support the FDO interface
      return base::MakeUnique<PowerSaveBlockerFDO>(type, description);
    case base::nix::DESKTOP_ENVIRONMENT_OTHER:
    case base::nix::DESKTOP_ENVIRONMENT_KDE3:
      return nullptr;
  }
}

} // namespace oxide
