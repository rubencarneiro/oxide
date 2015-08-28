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

#include <QtGlobal>

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"

#include "shared/browser/oxide_vibration_manager.h"

QT_BEGIN_NAMESPACE
class QFeedbackHapticsEffect;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class VibrationManager : public mojo::InterfaceImpl<device::VibrationManager> {
 public:
  static mojo::InterfaceImpl<device::VibrationManager>* Create();

  void Vibrate(int64 milliseconds) override;
  void Cancel() override;

 private:
  VibrationManager();
  ~VibrationManager() override;

  scoped_ptr<QFeedbackHapticsEffect> vibration_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_VIBRATION_MANAGER_H_

