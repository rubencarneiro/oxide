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

#include "source.h"

#include <QDateTime>
#include <QGeoCoordinate>
#include <QtGlobal>
#include <QTimer>

namespace {

double rand(double lbound, double ubound) {
  return qrand() * (ubound - lbound) / RAND_MAX + lbound;
}

QGeoPositionInfo randomPosition() {
  QGeoPositionInfo position;
  position.setTimestamp(QDateTime::currentDateTime());
  position.setCoordinate(QGeoCoordinate(rand(-90., 90.), rand(-180., 180.)));
  position.setAttribute(QGeoPositionInfo::HorizontalAccuracy, rand(0., 10000.));
  return position;
}

}

SourceMock::SourceMock(QObject* parent) : QGeoPositionInfoSource(parent) {
  qsrand(QDateTime::currentDateTime().toTime_t());
}

QGeoPositionInfo SourceMock::lastKnownPosition(
    bool fromSatellitePositioningMethodsOnly) const {
  return randomPosition();
}

QGeoPositionInfoSource::PositioningMethods
SourceMock::supportedPositioningMethods() const {
  return AllPositioningMethods;
}

int SourceMock::minimumUpdateInterval() const {
  return 1000;
}

QGeoPositionInfoSource::Error SourceMock::error() const {
  return NoError;
}

void SourceMock::startUpdates() {}

void SourceMock::stopUpdates() {}

void SourceMock::requestUpdate(int timeout) {
  Q_UNUSED(timeout);
  QTimer::singleShot((int) rand(1., 1000.), this, SLOT(sendUpdate()));
}

void SourceMock::sendUpdate() {
  QGeoPositionInfo update = randomPosition();
  Q_EMIT positionUpdated(update);
}
