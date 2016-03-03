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

#include "oxide_qquick_touch_handle_drawable.h"

#include <QDebug>
#include <QObject>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

#include "qt/quick/api/oxideqquicktouchselectioncontroller.h"
#include "qt/quick/api/oxideqquickwebview_p.h"

namespace oxide {
namespace qquick {

class TouchHandleDrawableContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(OxideQQuickTouchSelectionController::HandleOrientation orientation READ orientation NOTIFY orientationChanged FINAL)
  Q_PROPERTY(bool mirrorVertical READ mirrorVertical NOTIFY mirrorVerticalChanged FINAL)
  Q_PROPERTY(bool mirrorHorizontal READ mirrorHorizontal NOTIFY mirrorHorizontalChanged FINAL)
  Q_PROPERTY(qreal horizontalPaddingRatio READ horizontalPaddingRatio WRITE setHorizontalPaddingRatio NOTIFY horizontalPaddingRatioChanged)

 public:
  virtual ~TouchHandleDrawableContext() {}
  TouchHandleDrawableContext();

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

TouchHandleDrawableContext::TouchHandleDrawableContext()
    : orientation_(
        OxideQQuickTouchSelectionController::HandleOrientationUndefined)
    , mirror_vertical_(false)
    , mirror_horizontal_(false)
    , horizontal_padding_ratio_(0.0) {}

OxideQQuickTouchSelectionController::HandleOrientation
TouchHandleDrawableContext::orientation() const {
  return orientation_;
}

void TouchHandleDrawableContext::setOrientation(
    OxideQQuickTouchSelectionController::HandleOrientation orientation) {
  if (orientation_ != orientation) {
    orientation_ = orientation;
    Q_EMIT orientationChanged();
  }
}

bool TouchHandleDrawableContext::mirrorVertical() const {
  return mirror_vertical_;
}

void TouchHandleDrawableContext::setMirrorVertical(bool mirror) {
  if (mirror_vertical_ != mirror) {
    mirror_vertical_ = mirror;
    Q_EMIT mirrorVerticalChanged();
  }
}

bool TouchHandleDrawableContext::mirrorHorizontal() const {
  return mirror_horizontal_;
}

void TouchHandleDrawableContext::setMirrorHorizontal(bool mirror) {
  if (mirror_horizontal_ != mirror) {
    mirror_horizontal_ = mirror;
    Q_EMIT mirrorHorizontalChanged();
  }
}

qreal TouchHandleDrawableContext::horizontalPaddingRatio() const {
  return horizontal_padding_ratio_;
}

void TouchHandleDrawableContext::setHorizontalPaddingRatio(qreal ratio) {
  qreal bound = qBound(0.0, ratio, 1.0);
  if (horizontal_padding_ratio_ != bound) {
    horizontal_padding_ratio_ = bound;
    Q_EMIT horizontalPaddingRatioChanged();
  }
}

TouchHandleDrawable::~TouchHandleDrawable() {}

void TouchHandleDrawable::SetEnabled(bool enabled) {
  if (!item_.isNull()) {
    item_->setVisible(enabled);
  }
}

void TouchHandleDrawable::SetOrientation(Orientation orientation,
                                         bool mirror_vertical,
                                         bool mirror_horizontal) {
  if (!context_.isNull()) {
    TouchHandleDrawableContext* context =
        qobject_cast<TouchHandleDrawableContext*>(context_->contextObject());
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

void TouchHandleDrawable::SetOrigin(const QPointF& origin) {
  if (!item_.isNull()) {
    item_->setX(origin.x());
    item_->setY(origin.y());
  }
}

void TouchHandleDrawable::SetAlpha(float alpha) {
  if (!item_.isNull()) {
    item_->setOpacity(alpha);
  }
}

QRectF TouchHandleDrawable::GetVisibleBounds() const {
  if (item_.isNull()) {
    return QRectF();
  }

  return QRectF(item_->x(), item_->y(), item_->width(), item_->height());
}

float TouchHandleDrawable::GetDrawableHorizontalPaddingRatio() const {
  if (context_.isNull()) {
    return 0.0f;
  }

  TouchHandleDrawableContext* context =
      qobject_cast<TouchHandleDrawableContext*>(context_->contextObject());
  return context->horizontalPaddingRatio();
}

void TouchHandleDrawable::instantiateComponent() {
  if (!parent_) {
    qWarning() <<
        "TouchHandleDrawable: "
        "Can't instantiate after the view has gone";
    return;
  }

  TouchHandleDrawableContext* contextObject =
      new TouchHandleDrawableContext();
  QQmlComponent* component = controller_->handle();
  if (!component) {
    qWarning() <<
        "TouchHandleDrawable: Content requested a touch handle "
        "drawable, but the application hasn't provided one";
    delete contextObject;
    return;
  }

  QQmlContext* baseContext = component->creationContext();
  if (!baseContext) {
    baseContext = QQmlEngine::contextForObject(parent_);
  }
  context_.reset(new QQmlContext(baseContext));

  context_->setContextObject(contextObject);
  contextObject->setParent(context_.data());

  item_.reset(
      qobject_cast<QQuickItem*>(component->beginCreate(context_.data())));
  if (!item_) {
    qWarning() <<
        "TouchHandleDrawable: Failed to create instance of "
        "Qml touch selection handle component";
    context_.reset();
    return;
  }

  // This is a bit hacky - not sure what we'll do here if we introduce
  // other items that can use a ContentView and which support custom UI
  // components
  OxideQQuickWebView* web_view = qobject_cast<OxideQQuickWebView*>(parent_);
  if (web_view) {
    OxideQQuickWebViewPrivate::get(web_view)
        ->addAttachedPropertyTo(item_.data());
  }
  item_->setParentItem(parent_);
  component->completeCreate();
}

TouchHandleDrawable::TouchHandleDrawable(
    QQuickItem* parent,
    OxideQQuickTouchSelectionController* controller)
    : parent_(parent),
      controller_(controller) {
  instantiateComponent();

  connect(controller, SIGNAL(handleChanged()), SLOT(handleComponentChanged()));
}

void TouchHandleDrawable::handleComponentChanged() {
  bool visible = item_.isNull() ? false : item_->isVisible();
  OxideQQuickTouchSelectionController::HandleOrientation orientation =
      OxideQQuickTouchSelectionController::HandleOrientationUndefined;
  bool mirror_vertical = false;
  bool mirror_horizontal = false;
  if (!context_.isNull()) {
    TouchHandleDrawableContext* context =
        qobject_cast<TouchHandleDrawableContext*>(context_->contextObject());
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
    TouchHandleDrawableContext* context =
        qobject_cast<TouchHandleDrawableContext*>(
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

#include "oxide_qquick_touch_handle_drawable.moc"
