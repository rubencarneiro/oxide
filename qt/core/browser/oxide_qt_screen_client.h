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

#ifndef _OXIDE_QT_CORE_BROWSER_SCREEN_CLIENT_H_
#define _OXIDE_QT_CORE_BROWSER_SCREEN_CLIENT_H_

#include <QObject>
#include <QtGlobal>

#include "base/macros.h"
#include "base/synchronization/lock.h"

#include "shared/browser/oxide_screen_client.h"

namespace oxide {
namespace qt {

class ScreenClient : public QObject,
                     public oxide::ScreenClient {
  Q_OBJECT

 public:
  ScreenClient();
  ~ScreenClient() override;

 private Q_SLOTS:
  void OnScreenGeometryChanged(const QRect& geometry);
  void OnScreenOrientationChanged(Qt::ScreenOrientation orientation);

 private:
  void UpdatePrimaryDisplay();

  // oxide::ScreenClient implementation
  gfx::Display GetPrimaryDisplay() override;
  gfx::Point GetCursorScreenPoint() override;

  base::Lock primary_display_lock_;
  gfx::Display primary_display_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_SCREEN_CLIENT_H_
