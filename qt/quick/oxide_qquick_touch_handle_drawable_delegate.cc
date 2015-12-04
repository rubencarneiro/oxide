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
  Q_PROPERTY(OxideQQuickTouchSelectionController::HandleOrientation orientation READ orientation NOTIFY orientationChanged FINAL)
  Q_PROPERTY(bool mirrorVertical READ mirrorVertical NOTIFY mirrorVerticalChanged FINAL)
  Q_PROPERTY(bool mirrorHorizontal READ mirrorHorizontal NOTIFY mirrorHorizontalChanged FINAL)
  Q_PROPERTY(qreal horizontalPaddingRatio READ horizontalPaddingRatio WRITE setHorizontalPaddingRatio NOTIFY horizontalPaddingRatioChanged)

 public:
  virtual ~TouchHandleDrawableDelegateContext() {}
  TouchHandleDrawableDelegateContext();

  OxideQQuickTouchSelectionController::HandleOrientation orientation() const;
  void setOrientation(
      OxideQQuickTouchSelectionController::HandleOrientation orientation);

  bool mirrorVertical() const;
  void setMirrorVertical(bool mirror);

  bool mirrorHorizontal() const;
  void setMirrorHorizontal(bool mirror);

  qreal horizontalPaddingRatio() const;
  void setHorizontalPaddingRatio(qreal ratio);

 Q_SIGNALS:
  void orientationChanged() const;
  void mirrorVerticalChanged() const;
  void mirrorHorizontalChanged() const;
  void horizontalPaddingRatioChanged() const;

 private:
  OxideQQuickTouchSelectionController::HandleOrientation orientation_;
  bool mirror_vertical_;
  bool mirror_horizontal_;
  qreal horizontal_padding_ratio_;
};

TouchHandleDrawableDelegateContext::TouchHandleDrawableDelegateContext()
    : orientation_(
        OxideQQuickTouchSelectionController::HandleOrientationUndefined)
    , mirror_vertical_(false)
    , mirror_horizontal_(false)
    , horizontal_padding_ratio_(0.0) {}

OxideQQuickTouchSelectionController::HandleOrientation
TouchHandleDrawableDelegateContext::orientation() const {
  return orientation_;
}

void TouchHandleDrawableDelegateContext::setOrientation(
    OxideQQuickTouchSelectionController::HandleOrientation orientation) {
  if (orientation_ != orientation) {
    orientation_ = orientation;
    Q_EMIT orientationChanged();
  }
}

bool TouchHandleDrawableDelegateContext::mirrorVertical() const {
  return mirror_vertical_;
}

void TouchHandleDrawableDelegateContext::setMirrorVertical(bool mirror) {
  if (mirror_vertical_ != mirror) {
    mirror_vertical_ = mirror;
    Q_EMIT mirrorVerticalChanged();
  }
}

bool TouchHandleDrawableDelegateContext::mirrorHorizontal() const {
  return mirror_horizontal_;
}

void TouchHandleDrawableDelegateContext::setMirrorHorizontal(bool mirror) {
  if (mirror_horizontal_ != mirror) {
    mirror_horizontal_ = mirror;
    Q_EMIT mirrorHorizontalChanged();
  }
}

qreal TouchHandleDrawableDelegateContext::horizontalPaddingRatio() const {
  return horizontal_padding_ratio_;
}

void TouchHandleDrawableDelegateContext::setHorizontalPaddingRatio(
    qreal ratio) {
  qreal bound = qBound(0.0, ratio, 1.0);
  if (horizontal_padding_ratio_ != bound) {
    horizontal_padding_ratio_ = bound;
    Q_EMIT horizontalPaddingRatioChanged();
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
        qobject_cast<TouchHandleDrawableDelegateContext*>(
            context_->contextObject());
    if (context) {
      OxideQQuickTouchSelectionController::HandleOrientation o;
      switch (orientation) {
        case Orientation::Left:
          o = OxideQQuickTouchSelectionController::HandleOrientationLeft;
          break;
        case Orientation::Center:
          o = OxideQQuickTouchSelectionController::HandleOrientationCenter;
          break;
        case Orientation::Right:
          o = OxideQQuickTouchSelectionController::HandleOrientationRight;
          break;
        case Orientation::Undefined:
          o = OxideQQuickTouchSelectionController::HandleOrientationUndefined;
          break;
        default:
          Q_UNREACHABLE();
      }
      context->setOrientation(o);
      context->setMirrorVertical(mirror_vertical);
      context->setMirrorHorizontal(mirror_horizontal);
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
  if (context_.isNull()) {
    return 0.0f;
  }

  TouchHandleDrawableDelegateContext* context =
      qobject_cast<TouchHandleDrawableDelegateContext*>(
          context_->contextObject());
  return context->horizontalPaddingRatio();
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
  OxideQQuickTouchSelectionController::HandleOrientation orientation =
      OxideQQuickTouchSelectionController::HandleOrientationUndefined;
  bool mirror_vertical = false;
  bool mirror_horizontal = false;
  if (!context_.isNull()) {
    TouchHandleDrawableDelegateContext* context =
        qobject_cast<TouchHandleDrawableDelegateContext*>(
          context_->contextObject());
    orientation = context->orientation();
    mirror_vertical = context->mirrorVertical();
    mirror_horizontal = context->mirrorHorizontal();
  }
  QPointF position(item_.isNull() ? 0.0 : item_->x(),
                   item_.isNull() ? 0.0 : item_->y());
  qreal opacity = item_.isNull() ? 0.0 : item_->opacity();

  item_.reset();
  context_.reset();
  instantiateComponent();

  if (!item_.isNull()) {
    SetEnabled(visible);
    TouchHandleDrawableDelegateContext* context =
        qobject_cast<TouchHandleDrawableDelegateContext*>(
          context_->contextObject());
    context->setOrientation(orientation);
    context->setMirrorVertical(mirror_vertical);
    context->setMirrorHorizontal(mirror_horizontal);
    SetOrigin(position);
    SetAlpha(opacity);
  }
}

} // namespace qquick
} // namespace oxide

#include "oxide_qquick_touch_handle_drawable_delegate.moc"
