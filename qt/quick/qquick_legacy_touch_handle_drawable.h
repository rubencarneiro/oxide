// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015-2016 Canonical Ltd.

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

#ifndef _OXIDE_QT_QUICK_LEGACY_TOUCH_HANDLE_DRAWABLE_H_
#define _OXIDE_QT_QUICK_LEGACY_TOUCH_HANDLE_DRAWABLE_H_

#include <memory>

#include <QPointer>
#include <QtGlobal>

#include "qt/core/glue/touch_handle_drawable.h"

QT_BEGIN_NAMESPACE
class QQmlComponent;
class QQmlContext;
class QQuickItem;
QT_END_NAMESPACE

class OxideQQuickTouchSelectionController;

namespace oxide {
namespace qquick {

class LegacyTouchHandleDrawable
    : public QObject,
      public qt::TouchHandleDrawable {
  Q_OBJECT

 public:
  LegacyTouchHandleDrawable(QQuickItem* parent,
                            OxideQQuickTouchSelectionController* controller);

 private Q_SLOTS:
  void HandleComponentChanged();

 private:
  ~LegacyTouchHandleDrawable() override;

  void InstantiateComponent();

  // qt::TouchHandleDrawable implementation
  void SetEnabled(bool enabled) override;
  void SetOrientation(Orientation orientation,
                      bool mirror_vertical,
                      bool mirror_horizontal) override;
  void SetOrigin(const QPointF& origin) override;
  void SetAlpha(float alpha) override;
  QRectF GetVisibleBounds() const override;
  float GetDrawableHorizontalPaddingRatio() const override;

  QPointer<QQuickItem> parent_;
  QPointer<OxideQQuickTouchSelectionController> controller_;

  std::unique_ptr<QQmlContext> context_;
  std::unique_ptr<QQuickItem> item_;
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QT_QUICK_LEGACY_TOUCH_HANDLE_DRAWABLE_H_
