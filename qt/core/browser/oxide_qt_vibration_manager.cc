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

#include "base/logging.h"
#include "mojo/public/cpp/bindings/strong_binding.h"

#include <QFeedbackHapticsEffect>

namespace oxide {
namespace qt {

namespace {
const double kVibrationIntensity = 1.0;
}

// static
void VibrationManager::Create(
    mojo::InterfaceRequest<device::VibrationManager> request) {
  mojo::MakeStrongBinding(base::MakeUnique<VibrationManager>(),
                          std::move(request));
}

VibrationManager::VibrationManager()
    : vibration_(new QFeedbackHapticsEffect()) {}

VibrationManager::~VibrationManager() = default;

void VibrationManager::Vibrate(int64_t milliseconds,
                               const VibrateCallback& callback) {
  // This comes directly from the renderer - don't trust the value it gives us
  int64_t sanitized_duration =
      std::max(int64_t(1), std::min(milliseconds, int64_t(10000)));

  vibration_->stop();

  vibration_->setIntensity(kVibrationIntensity);
  vibration_->setDuration(sanitized_duration);
  vibration_->start();

  callback.Run();
}

void VibrationManager::Cancel(const CancelCallback& callback) {
  vibration_->stop();
  callback.Run();
}

} // namespace qt
} // namespace oxide

