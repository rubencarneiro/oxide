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

#include <cfloat>

#include <QGeoCoordinate>
#include <QGeoPositionInfo>
#include <QMetaObject>
#include <QThread>

#include "base/bind.h"
#include "base/location.h"
#include "base/logging.h"

Q_DECLARE_METATYPE(QGeoPositionInfo)

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
    position.accuracy = DBL_MAX;
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

LocationProvider::LocationProvider() :
    proxy_(base::MessageLoopProxy::current()),
    is_permission_granted_(false),
    source_(NULL),
    worker_thread_(NULL) {}

LocationProvider::~LocationProvider() {
  StopProvider();
}

bool LocationProvider::StartProvider(bool high_accuracy) {
  Q_UNUSED(high_accuracy);
  if (worker_thread_) {
    return worker_thread_->isRunning();
  }
  DCHECK(!source_);
  source_ = new LocationSource(this);
  worker_thread_ = new QThread();
  source_->moveToThread(worker_thread_);
  QObject::connect(worker_thread_, SIGNAL(finished()),
                   source_, SLOT(deleteLater()));
  worker_thread_->start();
  QMetaObject::invokeMethod(source_, "initOnWorkerThread", Qt::QueuedConnection);
  if (is_permission_granted_) {
    QMetaObject::invokeMethod(source_, "startUpdates", Qt::QueuedConnection);
  }
  if (worker_thread_->isRunning()) {
    return true;
  } else {
    delete worker_thread_;
    worker_thread_ = NULL;
    delete source_;
    source_ = NULL;
    return false;
  }
}

void LocationProvider::StopProvider() {
  if (worker_thread_) {
    worker_thread_->quit();
    worker_thread_->wait();
    delete worker_thread_;
    worker_thread_ = NULL;
  }
}

void LocationProvider::GetPosition(content::Geoposition* position) {
  DCHECK(position);
  *position = position_;
}

void LocationProvider::RequestRefresh() {
  if (is_permission_granted_ && source_) {
    QMetaObject::invokeMethod(source_, "requestUpdate", Qt::QueuedConnection);
  }
}

void LocationProvider::OnPermissionGranted() {
  if (!is_permission_granted_) {
    is_permission_granted_ = true;
    if (worker_thread_) {
      QMetaObject::invokeMethod(source_, "startUpdates", Qt::QueuedConnection);
    }
  }
}

void LocationProvider::cachePosition(const content::Geoposition& position) {
  position_ = position;
}

void LocationProvider::notifyCallbackOnGeolocationThread(
    const content::Geoposition& position) {
  if (position.Validate()) {
    proxy_->PostTask(FROM_HERE, base::Bind(&LocationProvider::cachePosition,
                                           base::Unretained(this), position));
  }
  proxy_->PostTask(FROM_HERE, base::Bind(&LocationProvider::NotifyCallback,
                                         base::Unretained(this), position));
}

LocationSource::LocationSource(LocationProvider* provider) :
    QObject(),
    provider_(provider),
    source_(NULL) {}

void LocationSource::initOnWorkerThread() {
  DCHECK(!source_);
  DCHECK_EQ(thread(), QThread::currentThread());
  source_ = QGeoPositionInfoSource::createDefaultSource(this);
  if (source_) {
    qRegisterMetaType<QGeoPositionInfo>();
    connect(source_, SIGNAL(positionUpdated(const QGeoPositionInfo&)),
            SLOT(positionUpdated(const QGeoPositionInfo&)));
    connect(source_, SIGNAL(error(QGeoPositionInfoSource::Error)),
            SLOT(error(QGeoPositionInfoSource::Error)));
  }
}

void LocationSource::startUpdates() const {
  if (source_) {
    source_->startUpdates();
  }
}

void LocationSource::requestUpdate() const {
  if (source_) {
    source_->requestUpdate();
  }
}

void LocationSource::positionUpdated(const QGeoPositionInfo& info) {
  if (info.isValid()) {
    provider_->notifyCallbackOnGeolocationThread(geopositionFromQt(info));
  } else {
    content::Geoposition error;
    error.error_code = content::Geoposition::ERROR_CODE_POSITION_UNAVAILABLE;
    provider_->notifyCallbackOnGeolocationThread(error);
  }
}

void LocationSource::error(QGeoPositionInfoSource::Error error) {
  content::Geoposition position;
  switch (error) {
    case QGeoPositionInfoSource::AccessError:
    case QGeoPositionInfoSource::ClosedError:
      position.error_code = content::Geoposition::ERROR_CODE_PERMISSION_DENIED;
      break;
    case QGeoPositionInfoSource::UnknownSourceError:
      position.error_code = content::Geoposition::ERROR_CODE_POSITION_UNAVAILABLE;
      break;
    case QGeoPositionInfoSource::NoError:
    default:
      return;
  }
  provider_->notifyCallbackOnGeolocationThread(position);
}

} // namespace qt
} // namespace oxide
