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
#include "oxide_qt_location_provider_p.h"

#include <cfloat>

#include <QGeoCoordinate>
#include <QGeoPositionInfo>
#include <QMetaObject>
#include <QtGlobal>
#include <QThread>

#include "base/bind.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/message_loop/message_loop_proxy.h"

Q_DECLARE_METATYPE(QGeoPositionInfo)

namespace oxide {
namespace qt {

static content::Geoposition geopositionFromQt(const QGeoPositionInfo& info) {
  content::Geoposition position;
  QGeoCoordinate coord = info.coordinate();
  position.latitude = coord.latitude();
  position.longitude = coord.longitude();
  position.altitude = coord.altitude();
  if (qIsNaN(position.altitude)) {
    // shield ourselves against invalid data
    position.altitude = 0;
  }
  if (info.hasAttribute(QGeoPositionInfo::HorizontalAccuracy)) {
    position.accuracy = info.attribute(QGeoPositionInfo::HorizontalAccuracy);
    if (qIsNaN(position.accuracy)) {
      // shield ourselves against invalid data
      position.accuracy = DBL_MAX;
    }
  } else {
    // accuracy is mandatory
    position.accuracy = DBL_MAX;
  }
  if (info.hasAttribute(QGeoPositionInfo::VerticalAccuracy)) {
    qreal accuracy = info.attribute(QGeoPositionInfo::VerticalAccuracy);
    if (!qIsNaN(accuracy)) {
      // shield ourselves against invalid data
      position.altitude_accuracy = accuracy;
    }
  }
  if (info.hasAttribute(QGeoPositionInfo::GroundSpeed)) {
    qreal speed = info.attribute(QGeoPositionInfo::GroundSpeed);
    if (!qIsNaN(speed)) {
      // shield ourselves against invalid data
      position.speed = speed;
    }
  }
  position.timestamp =
      base::Time::FromJsTime(info.timestamp().toMSecsSinceEpoch());
  return position;
}

LocationProvider::LocationProvider() :
    running_(false),
    proxy_(base::MessageLoopProxy::current()),
    is_permission_granted_(false),
    source_(NULL),
    worker_thread_(NULL) {}

LocationProvider::~LocationProvider() {
  StopProvider();
  if (worker_thread_) {
    worker_thread_->quit();
    worker_thread_->wait();
    delete worker_thread_;
  }
}

bool LocationProvider::StartProvider(bool high_accuracy) {
  Q_UNUSED(high_accuracy);
  if (!worker_thread_) {
    worker_thread_ = new QThread();
  }
  worker_thread_->start();
  if (!source_) {
    source_ = new LocationSource(this);
    source_->moveToThread(worker_thread_);
    QObject::connect(worker_thread_, SIGNAL(finished()),
                     source_, SLOT(deleteLater()));
    invokeOnWorkerThread("initOnWorkerThread");
  }
  running_ = true;
  if (is_permission_granted_) {
    invokeOnWorkerThread("startUpdates");
  }
  if (worker_thread_->isRunning()) {
    return true;
  } else {
    running_ = false;
    delete worker_thread_;
    worker_thread_ = NULL;
    delete source_;
    source_ = NULL;
    return false;
  }
}

void LocationProvider::StopProvider() {
  running_ = false;
  invokeOnWorkerThread("stopUpdates");
}

void LocationProvider::GetPosition(content::Geoposition* position) {
  DCHECK(position);
  *position = position_;
}

void LocationProvider::RequestRefresh() {
  if (is_permission_granted_) {
    invokeOnWorkerThread("requestUpdate");
  }
}

void LocationProvider::OnPermissionGranted() {
  if (!is_permission_granted_) {
    is_permission_granted_ = true;
    invokeOnWorkerThread("startUpdates");
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
  proxy_->PostTask(FROM_HERE, base::Bind(&LocationProvider::doNotifyCallback,
                                         base::Unretained(this), position));
}

void LocationProvider::doNotifyCallback(const content::Geoposition& position) {
  if (running_) {
    NotifyCallback(position);
  }
}

void LocationProvider::invokeOnWorkerThread(const char* method) const {
  if (source_) {
    QMetaObject::invokeMethod(source_, method, Qt::QueuedConnection);
  }
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

void LocationSource::stopUpdates() const {
  if (source_) {
    source_->stopUpdates();
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
