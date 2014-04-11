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

#ifndef OXIDE_Q_NEW_VIEW_REQUEST
#define OXIDE_Q_NEW_VIEW_REQUEST

#include <QObject>
#include <QRect>
#include <QRectF>
#include <QScopedPointer>
#include <QtGlobal>

class OxideQNewViewRequestPrivate;

class Q_DECL_EXPORT OxideQNewViewRequest Q_DECL_FINAL : public QObject {
  Q_OBJECT

  Q_PROPERTY(QRectF position READ positionF CONSTANT)
  Q_PROPERTY(Disposition disposition READ disposition CONSTANT)

  Q_ENUMS(Disposition)

  Q_DECLARE_PRIVATE(OxideQNewViewRequest)
  Q_DISABLE_COPY(OxideQNewViewRequest)

 public:

  enum Disposition {
    DispositionCurrentTab,
    DispositionNewForegroundTab,
    DispositionNewBackgroundTab,
    DispositionNewPopup,
    DispositionNewWindow
  };

  Q_DECL_HIDDEN OxideQNewViewRequest(const QRect& position,
                                     Disposition disposition);
  ~OxideQNewViewRequest();

  QRect position() const;
  QRectF positionF() const;
  Disposition disposition() const;

 private:
  QScopedPointer<OxideQNewViewRequestPrivate> d_ptr;
};

#endif // OXIDE_Q_NEW_VIEW_REQUEST
