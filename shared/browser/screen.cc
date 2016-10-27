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

#include "screen.h"

#include <string>

#include "base/environment.h"
#include "base/logging.h"

#include "display_form_factor.h"
#include "hybris_utils.h"
#include "screen_observer.h"
#include "shell_mode.h"

namespace oxide {

namespace {
Screen* g_instance;
}

Screen::Screen() {
  DCHECK(!g_instance);
  g_instance = this;
}

void Screen::NotifyPrimaryDisplayChanged() {
  for (auto& observer : observers_) {
    observer.OnPrimaryDisplayChanged();
  }
}

void Screen::NotifyDisplayAdded(const display::Display& display) {
  for (auto& observer : observers_) {
    observer.OnDisplayAdded(display);
  }
}

void Screen::NotifyDisplayRemoved(const display::Display& display) {
  for (auto& observer : observers_) {
    observer.OnDisplayRemoved(display);
  }
}

void Screen::NotifyDisplayPropertiesChanged(const display::Display& display) {
  for (auto& observer : observers_) {
    observer.OnDisplayPropertiesChanged(display);
  }
}

void Screen::NotifyShellModeChanged() {
  for (auto& observer : observers_) {
    observer.OnShellModeChanged();
  }
}

void Screen::AddObserver(ScreenObserver* observer) {
  DCHECK(thread_checker_.CalledOnValidThread());
  observers_.AddObserver(observer);
}

void Screen::RemoveObserver(ScreenObserver* observer) {
  DCHECK(thread_checker_.CalledOnValidThread());
  observers_.RemoveObserver(observer);
}

// static
Screen* Screen::GetInstance() {
  DCHECK(g_instance);
  DCHECK(g_instance->thread_checker_.CalledOnValidThread());
  return g_instance;
}

Screen::~Screen() {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK_EQ(g_instance, this);
  for (auto& observer : observers_) {
    observer.OnScreenDestruction();
  }
  g_instance = nullptr;
}

DisplayFormFactor Screen::GetDisplayFormFactor(
    const display::Display& display) {
  DCHECK(thread_checker_.CalledOnValidThread());

#if defined(ENABLE_HYBRIS)
  if (HybrisUtils::GetInstance()->HasDeviceProperties() &&
      display.id() == GetPrimaryDisplay().id()) {
    // Ubuntu on phones and tablets currently uses an Android kernel and EGL
    // stack. If we detect these, assume that the primary display is a mobile
    // display
    return DisplayFormFactor::Mobile;
  }
#endif

  // If this is not an Ubuntu mobile device, assume desktop for now
  return DisplayFormFactor::Monitor;
}

// static
ShellMode Screen::GetShellMode() {
  std::unique_ptr<base::Environment> env = base::Environment::Create();
  std::string override_mode;
  if (env->GetVar("OXIDE_FORCE_SHELL_MODE", &override_mode)) {
    if (override_mode == "windowed") {
      return ShellMode::Windowed;
    } else if (override_mode == "non-windowed") {
      return ShellMode::NonWindowed;
    }
    LOG(WARNING) << "Unrecognized value for OXIDE_FORCE_SHELL_MODE";
  }

  // FIXME: This is based on the same thing that GetDisplayFormFactor does,
  //  but it's not really correct. And it needs to be dynamic
#if defined(ENABLE_HYBRIS)
  if (HybrisUtils::GetInstance()->HasDeviceProperties()) {
    return ShellMode::NonWindowed;
  }
#endif

  return ShellMode::Windowed;
}

} // namespace oxide
