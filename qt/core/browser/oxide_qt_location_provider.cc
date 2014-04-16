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

#include "oxide_qt_location_provider.h"

#include <QDebug>
#include <QGeoCoordinate>
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>

namespace oxide {
namespace qt {

static content::Geoposition geopositionFromQt(const QGeoPositionInfo& info) {
  content::Geoposition position;
  QGeoCoordinate coord = info.coordinate();
  position.latitude = coord.latitude();
  position.longitude = coord.longitude();
  position.altitude = coord.altitude();
  if (info.hasAttribute(QGeoPositionInfo::HorizontalAccuracy)) {
    position.accuracy = info.attribute(QGeoPositionInfo::HorizontalAccuracy);
  } else {
    // accuracy is mandatory
    position.accuracy = 1000; // XXX: which accuracy to set (in meters)?
  }
  if (info.hasAttribute(QGeoPositionInfo::VerticalAccuracy)) {
    position.altitude_accuracy =
        info.attribute(QGeoPositionInfo::VerticalAccuracy);
  }
  if (info.hasAttribute(QGeoPositionInfo::GroundSpeed)) {
    position.speed = info.attribute(QGeoPositionInfo::GroundSpeed);
  }
  position.timestamp =
      base::Time::FromJsTime(info.timestamp().toMSecsSinceEpoch());
  return position;
}

LocationProvider::LocationProvider(QObject* parent) :
    QObject(parent),
    is_permission_granted_(false),
    source_(NULL) {}

LocationProvider::~LocationProvider() {
  StopProvider();
}

bool LocationProvider::StartProvider(bool high_accuracy) {
  Q_UNUSED(high_accuracy);
  qDebug() << Q_FUNC_INFO << high_accuracy;
  if (!source_) {
    source_ = QGeoPositionInfoSource::createDefaultSource(this);
    if (!source_) {
      return false;
    }
    qDebug() << Q_FUNC_INFO << "default geolocation source created:" << source_->sourceName();
    connect(source_, SIGNAL(positionUpdated(const QGeoPositionInfo&)),
            SLOT(positionUpdated(const QGeoPositionInfo&)));
    connect(source_, SIGNAL(updateTimeout()), SLOT(updateTimeout()));
  }
  if (is_permission_granted_) {
    qDebug() << Q_FUNC_INFO << "starting geolocation updates";
    source_->startUpdates();
  }
  return true;
}

void LocationProvider::StopProvider() {
  qDebug() << Q_FUNC_INFO;
  if (source_) {
    qDebug() << Q_FUNC_INFO << "stopping geolocation updates";
    source_->stopUpdates();
    delete source_;
    source_ = NULL;
  }
}

void LocationProvider::GetPosition(content::Geoposition* position) {
  qDebug() << Q_FUNC_INFO;
  if (source_) {
    QGeoPositionInfo info = source_->lastKnownPosition();
    if (info.isValid()) {
      *position = geopositionFromQt(info);
    }
  }
}

void LocationProvider::RequestRefresh() {
  if (is_permission_granted_ && source_) {
    source_->requestUpdate();
  }
}

void LocationProvider::OnPermissionGranted() {
  qDebug() << Q_FUNC_INFO;
  if (!is_permission_granted_) {
    is_permission_granted_ = true;
    RequestRefresh();
  }
}

void LocationProvider::positionUpdated(const QGeoPositionInfo& info) {
  qDebug() << Q_FUNC_INFO << info;
  if (info.isValid()) {
    NotifyCallback(geopositionFromQt(info));
  }
}

void LocationProvider::updateTimeout() {
  qDebug() << Q_FUNC_INFO;
  content::Geoposition position;
  position.error_code = content::Geoposition::ERROR_CODE_TIMEOUT;
  NotifyCallback(position);
}

} // namespace qt
} // namespace oxide
