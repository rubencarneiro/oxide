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

#ifndef _OXIDE_QT_CORE_GLUE_TOUCH_HANDLE_DRAWABLE_DELEGATE_PROXY_H_
#define _OXIDE_QT_CORE_GLUE_TOUCH_HANDLE_DRAWABLE_DELEGATE_PROXY_H_

#include <QRectF>
#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QPointF;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class TouchHandleDrawableDelegateProxy {
 public:
  // Matches chromium’s ui::TouchHandleOrientation
  enum Orientation {
    Left,
    Center,
    Right,
    Undefined,
  };

  virtual ~TouchHandleDrawableDelegateProxy() {}

  virtual void SetEnabled(bool enabled) = 0;
  virtual void SetOrientation(Orientation orientation,
                              bool mirror_vertical,
                              bool mirror_horizontal) = 0;
  virtual void SetOrigin(const QPointF& origin) = 0;
  virtual void SetAlpha(float alpha) = 0;
  virtual QRectF GetVisibleBounds() const = 0;
  virtual float GetDrawableHorizontalPaddingRatio() const = 0;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_TOUCH_HANDLE_DRAWABLE_DELEGATE_PROXY_H_
