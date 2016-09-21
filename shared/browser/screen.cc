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
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/threading/thread_checker.h"

#include "display_form_factor.h"
#include "hybris_utils.h"
#include "screen_observer.h"
#include "shell_mode.h"

namespace oxide {

namespace {
base::LazyInstance<base::ThreadChecker> g_thread_checker =
    LAZY_INSTANCE_INITIALIZER;
Screen* g_instance;
}

Screen::Screen() {
  DCHECK(g_thread_checker.Get().CalledOnValidThread());
  DCHECK(!g_instance);
  g_instance = this;
}

void Screen::NotifyPrimaryDisplayChanged() {
  FOR_EACH_OBSERVER(ScreenObserver, observers_, OnPrimaryDisplayChanged());
}

void Screen::NotifyDisplayAdded(const display::Display& display) {
  FOR_EACH_OBSERVER(ScreenObserver, observers_, OnDisplayAdded(display));
}

void Screen::NotifyDisplayRemoved(const display::Display& display) {
  FOR_EACH_OBSERVER(ScreenObserver, observers_, OnDisplayRemoved(display));
}

void Screen::NotifyDisplayPropertiesChanged(const display::Display& display) {
  FOR_EACH_OBSERVER(ScreenObserver,
                    observers_,
                    OnDisplayPropertiesChanged(display));
}

void Screen::NotifyShellModeChanged() {
  FOR_EACH_OBSERVER(ScreenObserver, observers_, OnShellModeChanged());
}

void Screen::AddObserver(ScreenObserver* observer) {
  observers_.AddObserver(observer);
}

void Screen::RemoveObserver(ScreenObserver* observer) {
  observers_.RemoveObserver(observer);
}

// static
Screen* Screen::GetInstance() {
  DCHECK(g_thread_checker.Get().CalledOnValidThread());
  DCHECK(g_instance);
  return g_instance;
}

Screen::~Screen() {
  DCHECK_EQ(g_instance, this);
  FOR_EACH_OBSERVER(ScreenObserver, observers_, OnScreenDestruction());
  g_instance = nullptr;
}

DisplayFormFactor Screen::GetDisplayFormFactor(
    const display::Display& display) {
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
