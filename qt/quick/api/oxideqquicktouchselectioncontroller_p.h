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

#ifndef _OXIDE_QT_QUICK_API_TOUCH_SELECTION_CONTROLLER_H_
#define _OXIDE_QT_QUICK_API_TOUCH_SELECTION_CONTROLLER_H_

#include <QObject>
#include <QRectF>
#include <QString>
#include <QtGlobal>

#include "oxideqquickwebview_p.h"

class QQmlComponent;

class OxideQQuickTouchSelectionControllerPrivate;

class Q_DECL_EXPORT OxideQQuickTouchSelectionController : public QObject {
  Q_OBJECT

  Q_ENUMS(Orientation);

  Q_PROPERTY(bool active READ active NOTIFY activeChanged)
  Q_PROPERTY(QQmlComponent* handle READ handle WRITE setHandle NOTIFY handleChanged)
  Q_PROPERTY(QRectF bounds READ bounds NOTIFY boundsChanged)
  Q_PROPERTY(OxideQQuickWebView::EditCapabilities editFlags READ editFlags NOTIFY editFlagsChanged)
  Q_PROPERTY(QString selectedText READ selectedText NOTIFY selectedTextChanged)

  Q_DISABLE_COPY(OxideQQuickTouchSelectionController)
  Q_DECLARE_PRIVATE(OxideQQuickTouchSelectionController)

 public:
  Q_DECL_HIDDEN OxideQQuickTouchSelectionController(OxideQQuickWebView* view);
  virtual ~OxideQQuickTouchSelectionController();

  enum Orientation {
    OrientationLeft,
    OrientationCenter,
    OrientationRight,
    OrientationUndefined
  };

  bool active() const;
  void setActive(bool active);

  QQmlComponent* handle() const;
  void setHandle(QQmlComponent* handle);

  const QRectF& bounds() const;
  void setBounds(const QRectF& bounds);

  OxideQQuickWebView::EditCapabilities editFlags() const;
  void setEditFlags(OxideQQuickWebView::EditCapabilities flags);

  const QString& selectedText() const;
  void setSelectedText(const QString& selectedText);

 Q_SIGNALS:
  void activeChanged();
  void handleChanged();
  void boundsChanged();
  void editFlagsChanged();
  void selectedTextChanged();

 private:
  QScopedPointer<OxideQQuickTouchSelectionControllerPrivate> d_ptr;
};

#endif // _OXIDE_QT_QUICK_API_TOUCH_SELECTION_CONTROLLER_H_
