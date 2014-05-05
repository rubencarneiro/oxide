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
#include <QMutexLocker>

#include "base/bind.h"
#include "base/location.h"

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
    message_loop_(base::MessageLoop::current()),
    is_permission_granted_(false),
    worker_(NULL) {}

LocationProvider::~LocationProvider() {
  StopProvider();
}

bool LocationProvider::StartProvider(bool high_accuracy) {
  Q_UNUSED(high_accuracy);
  if (worker_) {
    return worker_->isRunning();
  }
  worker_ = new LocationWorkerThread(this, is_permission_granted_);
  worker_->start();
  if (worker_->isRunning()) {
    return true;
  } else {
    delete worker_;
    worker_ = NULL;
    return false;
  }
}

void LocationProvider::StopProvider() {
  if (worker_) {
    worker_->quit();
    worker_->wait();
    delete worker_;
    worker_ = NULL;
  }
}

void LocationProvider::GetPosition(content::Geoposition* position) {
  if (worker_) {
    // FIXME: the following call is done on an object that lives in a different
    // thread, this is potentially unsafe! See if it can be done asynchronously
    // on its thread and communicate the result back to this thread.
    QGeoPositionInfo info = worker_->lastKnownPosition();
    if (info.isValid()) {
      *position = geopositionFromQt(info);
    }
  }
}

void LocationProvider::RequestRefresh() {
  if (is_permission_granted_ && worker_) {
    QMetaObject::invokeMethod(worker_, SLOT(requestUpdate()),
                              Qt::QueuedConnection);
  }
}

void LocationProvider::OnPermissionGranted() {
  if (!is_permission_granted_) {
    is_permission_granted_ = true;
    RequestRefresh();
  }
}

void LocationProvider::notifyCallbackOnGeolocationThread(
    const content::Geoposition& position) {
  message_loop_->PostTask(FROM_HERE,
                          base::Bind(&LocationProvider::NotifyCallback,
                                     base::Unretained(this),
                                     position));
}

LocationSource::LocationSource(LocationProvider* provider, bool start) :
    QObject(),
    provider_(provider),
    source_(NULL) {
  source_ = QGeoPositionInfoSource::createDefaultSource(this);
  if (source_) {
    qRegisterMetaType<QGeoPositionInfo>();
    connect(source_, SIGNAL(positionUpdated(const QGeoPositionInfo&)),
            SLOT(positionUpdated(const QGeoPositionInfo&)));
    connect(source_, SIGNAL(error(QGeoPositionInfoSource::Error)),
            SLOT(error(QGeoPositionInfoSource::Error)));
    if (start) {
      source_->startUpdates();
    }
  }
}

QGeoPositionInfo LocationSource::lastKnownPosition() const {
  if (source_) {
    return source_->lastKnownPosition();
  } else {
    return QGeoPositionInfo();
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

LocationWorkerThread::LocationWorkerThread(LocationProvider* provider,
                                           bool start) :
    QThread(),
    provider_(provider),
    start_(start),
    source_(NULL) {}

void LocationWorkerThread::run() {
  mutex_.lock();
  source_ = new LocationSource(provider_, start_);
  mutex_.unlock();
  exec();
  mutex_.lock();
  delete source_;
  mutex_.unlock();
}

QGeoPositionInfo LocationWorkerThread::lastKnownPosition() {
  QMutexLocker locker(&mutex_);
  if (source_) {
    return source_->lastKnownPosition();
  } else {
    return QGeoPositionInfo();
  }
}

void LocationWorkerThread::requestUpdate() {
  QMutexLocker locker(&mutex_);
  if (source_) {
    source_->requestUpdate();
  }
}

} // namespace qt
} // namespace oxide
