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

#ifndef OXIDE_QTQUICK_LOCATION_BAR_CONTROLLER
#define OXIDE_QTQUICK_LOCATION_BAR_CONTROLLER

#include <QtCore/QObject>
#include <QtCore/QtGlobal>

#include <OxideQtQuick/oxideqquickglobal.h>

class OxideQQuickLocationBarControllerPrivate;
class OxideQQuickWebView;

class OXIDE_QTQUICK_EXPORT OxideQQuickLocationBarController : public QObject {
  Q_OBJECT

  Q_PROPERTY(qreal height READ height WRITE setHeight NOTIFY heightChanged)
  Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
  Q_PROPERTY(bool animated READ animated WRITE setAnimated NOTIFY animatedChanged REVISION 1)

  Q_PROPERTY(qreal offset READ offset NOTIFY offsetChanged)
  Q_PROPERTY(qreal contentOffset READ contentOffset NOTIFY contentOffsetChanged)

  Q_ENUMS(Mode)

  Q_DISABLE_COPY(OxideQQuickLocationBarController)
  Q_DECLARE_PRIVATE(OxideQQuickLocationBarController)

 public:

  enum Mode {
    ModeAuto,
    ModeShown,
    ModeHidden
  };

  ~OxideQQuickLocationBarController() Q_DECL_OVERRIDE;

  qreal height() const;
  void setHeight(qreal height);

  Mode mode() const;
  void setMode(Mode mode);

  bool animated() const;
  void setAnimated(bool animated);

  qreal offset() const;
  qreal contentOffset() const;

 public Q_SLOTS:
  Q_REVISION(1) void show(bool animate);
  Q_REVISION(1) void hide(bool animate);

 Q_SIGNALS:
  void heightChanged();
  void modeChanged();
  Q_REVISION(1) void animatedChanged();
  void offsetChanged();
  void contentOffsetChanged();

 private:
  friend class OxideQQuickWebView;
  Q_DECL_HIDDEN OxideQQuickLocationBarController(OxideQQuickWebView* view);

  QScopedPointer<OxideQQuickLocationBarControllerPrivate> d_ptr;
};

#endif // OXIDE_QTQUICK_LOCATION_BAR_CONTROLLER
