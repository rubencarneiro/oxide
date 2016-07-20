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

#ifndef _OXIDE_QT_TESTS_MOCK_QPA_SHIMAPI_H_
#define _OXIDE_QT_TESTS_MOCK_QPA_SHIMAPI_H_

#include <QObject>
#include <Qt>

QT_BEGIN_NAMESPACE
class QRect;
class QScreen;
QT_END_NAMESPACE

class ShimApi : public QObject {
  Q_OBJECT

 public:
  Q_INVOKABLE void resetScreens();

  Q_INVOKABLE void overrideScreenGeometry(QScreen* screen,
                                          const QRect& geometry,
                                          const QRect& available_geometry);

  Q_INVOKABLE void setScreenOrientation(QScreen* screen,
                                        Qt::ScreenOrientation orientation);
};

#endif // _OXIDE_QT_TESTS_MOCK_QPA_SHIMAPI_H_
