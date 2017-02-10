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

#ifndef _OXIDE_QT_UITK_LIB_TOUCH_HANDLE_DRAWABLE_H_
#define _OXIDE_QT_UITK_LIB_TOUCH_HANDLE_DRAWABLE_H_

#include "qt/core/glue/touch_handle_drawable.h"

#include <memory>
#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QQuickItem;
QT_END_NAMESPACE

namespace oxide {
namespace uitk {

class TouchHandleDrawable : public qt::TouchHandleDrawable {
 public:
  ~TouchHandleDrawable() override;
  static std::unique_ptr<TouchHandleDrawable> Create(QQuickItem* parent);

 private:
  TouchHandleDrawable();
  bool Init(QQuickItem* parent);

  // qt::TouchHandleDrawable implementation
  void SetEnabled(bool enabled) override;
  void SetOrientation(Orientation orientation,
                      bool mirror_vertical,
                      bool mirror_horizontal) override;
  void SetOrigin(const QPointF& origin) override;
  void SetAlpha(float alpha) override;
  QRectF GetVisibleBounds() const override;
  float GetDrawableHorizontalPaddingRatio() const override;

  std::unique_ptr<QQuickItem> item_;
};

} // namespace uitk
} // namespace oxide

#endif // _OXIDE_QT_UITK_LIB_TOUCH_HANDLE_DRAWABLE_H_
