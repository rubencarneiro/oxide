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

#include "uitk_touch_handle_drawable.h"

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickItem>
#include <QRectF>
#include <QtDebug>

namespace oxide {
namespace uitk {

TouchHandleDrawable::TouchHandleDrawable() = default;

bool TouchHandleDrawable::Init(QQuickItem* parent) {
  QQmlEngine* engine = qmlEngine(parent);
  if (!engine) {
    qWarning() <<
        "uitk::TouchHandleDrawable: Failed to create touch handle - cannot "
        "determine QQmlEngine for parent item";
    return false;
  }

  QQmlComponent component(engine);
  component.loadUrl(QUrl("qrc:///TouchHandle.qml"));

  if (component.isError()) {
    qCritical() <<
        "uitk::TouchHandleDrawable: Failed to initialize touch handle "
        "component because of the following errors: ";
    for (const auto& error : component.errors()) {
      qCritical() << error;
    }
    return false;
  }

  Q_ASSERT(component.isReady());

  QObject* handle = component.beginCreate(engine->rootContext());
  if (!handle) {
    qCritical() <<
        "uitk::TouchHandleDrawable: Failed to create touch handle instance";
    return false;
  }

  item_.reset(qobject_cast<QQuickItem*>(handle));
  if (!item_) {
    qCritical() <<
        "uitk::TouchHandleDrawable: Touch handle instance is not a QQuickItem";
    delete handle;
    return false;
  }

  item_->setParentItem(parent);

  component.completeCreate();

  return true;
}

void TouchHandleDrawable::SetEnabled(bool enabled) {
  item_->setVisible(enabled);
}

void TouchHandleDrawable::SetOrientation(Orientation orientation,
                                         bool mirror_vertical,
                                         bool mirror_horizontal) {}

void TouchHandleDrawable::SetOrigin(const QPointF& origin) {
  item_->setX(origin.x());
  item_->setY(origin.y());
}

void TouchHandleDrawable::SetAlpha(float alpha) {
  item_->setOpacity(alpha);
}

QRectF TouchHandleDrawable::GetVisibleBounds() const {
  return QRectF(item_->x(), item_->y(), item_->width(), item_->height());
}

float TouchHandleDrawable::GetDrawableHorizontalPaddingRatio() const {
  return 0.5f;
}

TouchHandleDrawable::~TouchHandleDrawable() = default;

// static
std::unique_ptr<TouchHandleDrawable> TouchHandleDrawable::Create(
    QQuickItem* parent) {
  std::unique_ptr<TouchHandleDrawable> handle(new TouchHandleDrawable());
  if (!handle->Init(parent)) {
    return nullptr;
  }
  return std::move(handle);
}

} // namespace uitk
} // namespace oxide
