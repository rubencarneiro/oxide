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

#ifndef _OXIDE_QT_CORE_BROWSER_SCREEN_H_
#define _OXIDE_QT_CORE_BROWSER_SCREEN_H_

#include <map>

#include <QObject>
#include <QtGlobal>

#include "base/macros.h"

#include "qt/core/common/oxide_qt_export.h"
#include "shared/browser/screen.h"

QT_BEGIN_NAMESPACE
class QPlatformScreen;
class QScreen;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class OXIDE_QT_EXPORT Screen : public QObject,
                               public oxide::Screen {
  Q_OBJECT

 public:
  Screen();
  ~Screen() override;

  static Screen* GetInstance();

  display::Display DisplayFromQScreen(QScreen* screen);

  // oxide::Screen implementation
  display::Display GetPrimaryDisplay() override;
  std::vector<display::Display> GetAllDisplays() override;
  gfx::Point GetCursorScreenPoint() override;

 private Q_SLOTS:
  void OnScreenAdded(QScreen* screen);
  void OnScreenRemoved(QScreen* screen);
  void OnPrimaryScreenChanged(QScreen* screen);
  void OnPlatformScreenPropertyChanged(QPlatformScreen* screen,
                                       const QString& property_name);

 private:
  void UpdateDisplayForScreen(QScreen* screen, bool notify);

  std::map<QScreen*, display::Display> displays_;

  display::Display* primary_display_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_SCREEN_H_
