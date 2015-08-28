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

#include "oxide_vibration_manager.h"

#include "base/basictypes.h"
#include "base/bind.h"

namespace oxide {

namespace {

class VibrationManagerOxideImpl : public mojo::InterfaceImpl<device::VibrationManager> {
 public:
  static VibrationManagerOxideImpl* Create() {
    return new VibrationManagerOxideImpl();
  }

  void Vibrate(int64 milliseconds) override {}
  void Cancel() override {}

 private:
  VibrationManagerOxideImpl() {}
  ~VibrationManagerOxideImpl() override {}
};

VibrationManagerOxideFactory g_vibration_manager_factory =
  nullptr;

}  // namespace

void CreateVibrationManager(
      mojo::InterfaceRequest<device::VibrationManager> request) {
  if (g_vibration_manager_factory) {
    BindToRequest(g_vibration_manager_factory(), &request);
  }
  else {
    BindToRequest(VibrationManagerOxideImpl::Create(), &request);
  }
}

void SetVibrationManagerFactory(VibrationManagerOxideFactory factory) {
  g_vibration_manager_factory = factory;
}

}  // namespace oxide
