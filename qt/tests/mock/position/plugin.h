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

#ifndef _OXIDE_QT_TESTS_MOCK_POSITION_PLUGIN_H_
#define _OXIDE_QT_TESTS_MOCK_POSITION_PLUGIN_H_

#include <QGeoPositionInfoSourceFactory>
#include <QObject>

class PositionMock : public QObject, public QGeoPositionInfoSourceFactory
{
  Q_OBJECT

  Q_PLUGIN_METADATA(IID "org.qt-project.qt.position.sourcefactory/5.0"
                    FILE "plugin.json")

  Q_INTERFACES(QGeoPositionInfoSourceFactory)

 public:
  QGeoPositionInfoSource* positionInfoSource(QObject* parent);
  QGeoSatelliteInfoSource* satelliteInfoSource(QObject* parent);
  QGeoAreaMonitorSource* areaMonitor(QObject* parent);
};

#endif // _OXIDE_QT_TESTS_MOCK_POSITION_PLUGIN_H_
