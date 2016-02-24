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

#ifndef OXIDE_QTQUICK_TOUCH_SELECTION_CONTROLLER
#define OXIDE_QTQUICK_TOUCH_SELECTION_CONTROLLER

#include <QtCore/QObject>
#include <QtCore/QRectF>
#include <QtCore/QString>
#include <QtCore/QtGlobal>

#include <OxideQtQuick/oxideqquickglobal.h>

QT_BEGIN_NAMESPACE
class QQmlComponent;
QT_END_NAMESPACE

class OxideQQuickTouchSelectionControllerPrivate;
class OxideQQuickWebView;
class OxideQQuickWebViewPrivate;

namespace oxide {
namespace qquick {
class ContentsView;
}
}

class OXIDE_QTQUICK_EXPORT OxideQQuickTouchSelectionController
    : public QObject {
  Q_OBJECT

  Q_ENUMS(HandleOrientation);

  Q_PROPERTY(bool active READ active NOTIFY activeChanged)
  Q_PROPERTY(QQmlComponent* handle READ handle WRITE setHandle NOTIFY handleChanged)
  Q_PROPERTY(QRectF bounds READ bounds NOTIFY boundsChanged)

  Q_DISABLE_COPY(OxideQQuickTouchSelectionController)
  Q_DECLARE_PRIVATE(OxideQQuickTouchSelectionController)

 public:
  ~OxideQQuickTouchSelectionController() Q_DECL_OVERRIDE;

  enum HandleOrientation {
    HandleOrientationLeft,
    HandleOrientationCenter,
    HandleOrientationRight,
    HandleOrientationUndefined
  };

  bool active() const;

  QQmlComponent* handle() const;
  void setHandle(QQmlComponent* handle);

  const QRectF& bounds() const;

 Q_SIGNALS:
  void activeChanged();
  void handleChanged();
  void boundsChanged();

 private:
  friend class OxideQQuickWebViewPrivate;
  friend class oxide::qquick::ContentsView;

  Q_DECL_HIDDEN OxideQQuickTouchSelectionController(OxideQQuickWebView* view);

  void onTouchSelectionChanged(bool active, const QRectF& bounds);

  QScopedPointer<OxideQQuickTouchSelectionControllerPrivate> d_ptr;
};

#endif // OXIDE_QTQUICK_TOUCH_SELECTION_CONTROLLER
