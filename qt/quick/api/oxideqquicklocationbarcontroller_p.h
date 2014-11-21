// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#ifndef _OXIDE_QT_QUICK_API_LOCATION_BAR_CONTROLLER_H_
#define _OXIDE_QT_QUICK_API_LOCATION_BAR_CONTROLLER_H_

#include <QObject>
#include <QtGlobal>

class OxideQQuickLocationBarControllerPrivate;
class OxideQQuickWebView;

class Q_DECL_EXPORT OxideQQuickLocationBarController : public QObject {
  Q_OBJECT

  Q_PROPERTY(qreal maxHeight READ maxHeight WRITE setMaxHeight NOTIFY maxHeightChanged)
  Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)

  Q_PROPERTY(qreal offset READ offset NOTIFY offsetChanged)
  Q_PROPERTY(qreal contentOffset READ contentOffset NOTIFY contentOffsetChanged)

  Q_PROPERTY(qreal height READ height WRITE setHeight NOTIFY heightChanged)

  Q_ENUMS(Mode)

  Q_DISABLE_COPY(OxideQQuickLocationBarController)
  Q_DECLARE_PRIVATE(OxideQQuickLocationBarController)

 public:

  enum Mode {
    ModeAuto,
    ModeShown,
    ModeHidden
  };

  Q_DECL_HIDDEN OxideQQuickLocationBarController(OxideQQuickWebView* view);
  virtual ~OxideQQuickLocationBarController();

  qreal maxHeight() const;
  void setMaxHeight(qreal height);

  Mode mode() const;
  void setMode(Mode mode);

  qreal offset() const;
  qreal contentOffset() const;

  qreal height() const;
  void setHeight(qreal height);

 Q_SIGNALS:
  void maxHeightChanged();
  void modeChanged();
  void offsetChanged();
  void contentOffsetChanged();
  void heightChanged();

 private:
  QScopedPointer<OxideQQuickLocationBarControllerPrivate> d_ptr;
};

#endif // _OXIDE_QT_QUICK_API_LOCATION_BAR_CONTROLLER_H_
