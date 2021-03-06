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

#ifndef _OXIDE_QT_TESTS_MOCK_POSITION_SOURCE_H_
#define _OXIDE_QT_TESTS_MOCK_POSITION_SOURCE_H_

#include <QGeoPositionInfoSource>
#include <QTimer>

class SourceMock : public QGeoPositionInfoSource
{
  Q_OBJECT

 public:
  SourceMock(QObject* parent = nullptr);

  QGeoPositionInfo lastKnownPosition(
      bool fromSatellitePositioningMethodsOnly = false) const;
  PositioningMethods supportedPositioningMethods() const;
  int minimumUpdateInterval() const;
  Error error() const;

 public Q_SLOTS:
  void startUpdates();
  void stopUpdates();
  void requestUpdate(int timeout = 0);

 private Q_SLOTS:
  void sendUpdate();
  void sendInvalidUpdate();

 private:
  QTimer timer_;
};

#endif // _OXIDE_QT_TESTS_MOCK_POSITION_SOURCE_H_
