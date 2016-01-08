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

#ifndef _OXIDE_QT_QUICK_TOUCH_HANDLE_DRAWABLE_H_
#define _OXIDE_QT_QUICK_TOUCH_HANDLE_DRAWABLE_H_

#include <QPointer>
#include <QScopedPointer>
#include <QtGlobal>

#include "qt/core/glue/oxide_qt_touch_handle_drawable_proxy.h"

class OxideQQuickWebView;

QT_BEGIN_NAMESPACE
class QQmlContext;
class QQuickItem;
QT_END_NAMESPACE

namespace oxide {
namespace qquick {

class TouchHandleDrawable :
    public QObject, public oxide::qt::TouchHandleDrawableProxy {
  Q_OBJECT

 public:
  TouchHandleDrawable(OxideQQuickWebView* view);

 private Q_SLOTS:
  void handleComponentChanged();

 private:
  ~TouchHandleDrawable() override;

  void instantiateComponent();

  // oxide::qt::TouchHandleDrawableProxy implementation
  void SetEnabled(bool enabled);
  void SetOrientation(Orientation orientation,
                      bool mirror_vertical,
                      bool mirror_horizontal);
  void SetOrigin(const QPointF& origin);
  void SetAlpha(float alpha);
  QRectF GetVisibleBounds() const;
  float GetDrawableHorizontalPaddingRatio() const;

  QPointer<OxideQQuickWebView> view_;
  QScopedPointer<QQmlContext> context_;
  QScopedPointer<QQuickItem> item_;
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QT_QUICK_TOUCH_HANDLE_DRAWABLE_H_
