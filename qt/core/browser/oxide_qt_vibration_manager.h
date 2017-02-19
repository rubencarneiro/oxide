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

#ifndef _OXIDE_QT_CORE_BROWSER_VIBRATION_MANAGER_H_
#define _OXIDE_QT_CORE_BROWSER_VIBRATION_MANAGER_H_

#include <memory>
#include <QtGlobal>

#include "base/macros.h"

#include "device/vibration/vibration_manager.mojom.h"

QT_BEGIN_NAMESPACE
class QFeedbackHapticsEffect;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class VibrationManager : public device::mojom::VibrationManager {
 public:
  static void Create(
      mojo::InterfaceRequest<device::mojom::VibrationManager> request);

  VibrationManager();
  ~VibrationManager() override;

  // device::mojom::VibrationManager implementation
  void Vibrate(int64_t milliseconds, const VibrateCallback& callback) override;
  void Cancel(const CancelCallback& callback) override;

 private:
  std::unique_ptr<QFeedbackHapticsEffect> vibration_;

  DISALLOW_COPY_AND_ASSIGN(VibrationManager);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_VIBRATION_MANAGER_H_

