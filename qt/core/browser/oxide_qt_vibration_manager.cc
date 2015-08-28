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

#include "oxide_qt_vibration_manager.h"

#include <QFeedbackHapticsEffect>

namespace oxide {
namespace qt {

namespace {
const double kVibrationIntensity = 1.0;
}

// static
mojo::InterfaceImpl<device::VibrationManager>* VibrationManager::Create() {
  return new VibrationManager();
}

VibrationManager::VibrationManager()
  : vibration_(new QFeedbackHapticsEffect()) {
}
VibrationManager::~VibrationManager() {}

void VibrationManager::Vibrate(int64 milliseconds) {
  Cancel();
  
  vibration_->setIntensity(kVibrationIntensity);
  vibration_->setDuration(milliseconds);
  vibration_->start();
}

void VibrationManager::Cancel() {
  vibration_->stop();
}

} // namespace qt
} // namespace oxide

