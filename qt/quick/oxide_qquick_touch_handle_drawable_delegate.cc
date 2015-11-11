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

#include "oxide_qquick_touch_handle_drawable_delegate.h"

#include <QDebug>
#include <QObject>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

#include "qt/quick/api/oxideqquicktouchselectioncontroller_p.h"
#include "qt/quick/api/oxideqquickwebview_p.h"
#include "qt/quick/api/oxideqquickwebview_p_p.h"

namespace oxide {
namespace qquick {

class TouchHandleDrawableDelegateContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(OxideQQuickTouchSelectionController::Orientation orientation READ orientation NOTIFY orientationChanged FINAL)

 public:
  virtual ~TouchHandleDrawableDelegateContext() {}
  TouchHandleDrawableDelegateContext();

  OxideQQuickTouchSelectionController::Orientation orientation() const;
  void setOrientation(
      OxideQQuickTouchSelectionController::Orientation orientation,
      bool mirror_vertical,
      bool mirror_horizontal);

 Q_SIGNALS:
  void orientationChanged() const;

 private:
  OxideQQuickTouchSelectionController::Orientation orientation_;
};

TouchHandleDrawableDelegateContext::TouchHandleDrawableDelegateContext()
    : orientation_(OxideQQuickTouchSelectionController::OrientationUndefined) {}

OxideQQuickTouchSelectionController::Orientation
TouchHandleDrawableDelegateContext::orientation() const {
  return orientation_;
}

void TouchHandleDrawableDelegateContext::setOrientation(
    OxideQQuickTouchSelectionController::Orientation orientation,
    bool mirror_vertical,
    bool mirror_horizontal) {
  Q_UNUSED(mirror_vertical);
  Q_UNUSED(mirror_horizontal);

  if (orientation_ != orientation) {
    orientation_ = orientation;
    Q_EMIT orientationChanged();
  }
}

TouchHandleDrawableDelegate::~TouchHandleDrawableDelegate() {}

void TouchHandleDrawableDelegate::SetEnabled(bool enabled) {
  if (!item_.isNull()) {
    item_->setVisible(enabled);
  }
}

void TouchHandleDrawableDelegate::SetOrientation(Orientation orientation,
                                                 bool mirror_vertical,
                                                 bool mirror_horizontal) {
  if (!context_.isNull()) {
    TouchHandleDrawableDelegateContext* context =
        qobject_cast<TouchHandleDrawableDelegateContext*>(context_->contextObject());
    if (context) {
      OxideQQuickTouchSelectionController::Orientation o;
      switch (orientation) {
        case Orientation::Left:
          o = OxideQQuickTouchSelectionController::OrientationLeft;
          break;
        case Orientation::Center:
          o = OxideQQuickTouchSelectionController::OrientationCenter;
          break;
        case Orientation::Right:
          o = OxideQQuickTouchSelectionController::OrientationRight;
          break;
        case Orientation::Undefined:
          o = OxideQQuickTouchSelectionController::OrientationUndefined;
          break;
        default:
          Q_UNREACHABLE();
      }
      context->setOrientation(o, mirror_vertical, mirror_horizontal);
    }
  }
}

void TouchHandleDrawableDelegate::SetOrigin(const QPointF& origin) {
  if (!item_.isNull()) {
    item_->setX(origin.x());
    item_->setY(origin.y());
  }
}

void TouchHandleDrawableDelegate::SetAlpha(float alpha) {
  if (!item_.isNull()) {
    item_->setOpacity(alpha);
  }
}

QRectF TouchHandleDrawableDelegate::GetVisibleBounds() const {
  if (item_.isNull()) {
    return QRectF();
  }

  return QRectF(item_->x(), item_->y(), item_->width(), item_->height());
}

float TouchHandleDrawableDelegate::GetDrawableHorizontalPaddingRatio() const {
  return 0.0f;
}

void TouchHandleDrawableDelegate::instantiateComponent() {
  if (view_.isNull()) {
    qWarning() <<
        "TouchHandleDrawableDelegate: "
        "Can't instantiate after the view has gone";
    return;
  }

  TouchHandleDrawableDelegateContext* contextObject =
      new TouchHandleDrawableDelegateContext();
  QQmlComponent* component = view_->touchSelectionController()->handle();
  if (!component) {
    qWarning() <<
        "TouchHandleDrawableDelegate: Content requested a touch handle "
        "drawable, but the application hasn't provided one";
    delete contextObject;
    return;
  }

  QQmlContext* baseContext = component->creationContext();
  if (!baseContext) {
    baseContext = QQmlEngine::contextForObject(view_.data());
  }
  context_.reset(new QQmlContext(baseContext));

  context_->setContextObject(contextObject);
  contextObject->setParent(context_.data());

  item_.reset(
      qobject_cast<QQuickItem*>(component->beginCreate(context_.data())));
  if (!item_) {
    qWarning() <<
        "TouchHandleDrawableDelegate: Failed to create instance of "
        "Qml touch selection handle component";
    context_.reset();
    return;
  }

  OxideQQuickWebViewPrivate::get(
      view_.data())->addAttachedPropertyTo(item_.data());
  item_->setParentItem(view_.data());
  component->completeCreate();
}

TouchHandleDrawableDelegate::TouchHandleDrawableDelegate(
    OxideQQuickWebView* view)
    : view_(view) {
  instantiateComponent();

  if (view) {
    connect(view->touchSelectionController(), SIGNAL(handleChanged()),
            SLOT(handleComponentChanged()));
  }
}

void TouchHandleDrawableDelegate::handleComponentChanged() {
  bool visible = item_.isNull() ? false : item_->isVisible();
  OxideQQuickTouchSelectionController::Orientation orientation =
      OxideQQuickTouchSelectionController::OrientationUndefined;
  if (!context_.isNull()) {
    orientation =
        qobject_cast<TouchHandleDrawableDelegateContext*>(
          context_->contextObject())->orientation();
  }
  QPointF position(item_.isNull() ? 0.0 : item_->x(),
                   item_.isNull() ? 0.0 : item_->y());
  qreal opacity = item_.isNull() ? 0.0 : item_->opacity();

  item_.reset();
  context_.reset();
  instantiateComponent();

  if (!item_.isNull()) {
    SetEnabled(visible);
    qobject_cast<TouchHandleDrawableDelegateContext*>(
        context_->contextObject())->setOrientation(orientation, false, false);
    SetOrigin(position);
    SetAlpha(opacity);
  }
}

} // namespace qquick
} // namespace oxide

#include "oxide_qquick_touch_handle_drawable_delegate.moc"
