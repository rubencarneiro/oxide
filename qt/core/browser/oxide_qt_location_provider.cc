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

#include <limits>

#include <QGeoCoordinate>
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#include <QScopedPointer>
#include <QThread>

#include "base/bind.h"
#include "base/float_util.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/platform_thread.h"
#include "content/public/browser/browser_thread.h"

Q_DECLARE_METATYPE(QGeoPositionInfo)
Q_DECLARE_METATYPE(QGeoPositionInfoSource::Error)

namespace oxide {
namespace qt {

static content::Geoposition geopositionFromQt(const QGeoPositionInfo& info) {
  content::Geoposition position;
  QGeoCoordinate coord = info.coordinate();
  position.latitude = coord.latitude();
  position.longitude = coord.longitude();
  position.altitude = coord.altitude();
  if (base::IsNaN(position.altitude)) {
    // shield ourselves against invalid data
    position.altitude = 0;
  }
  if (info.hasAttribute(QGeoPositionInfo::HorizontalAccuracy)) {
    position.accuracy = info.attribute(QGeoPositionInfo::HorizontalAccuracy);
    if (base::IsNaN(position.accuracy)) {
      // shield ourselves against invalid data
      position.accuracy = std::numeric_limits<double>::max();
    }
  } else {
    // accuracy is mandatory
    position.accuracy = std::numeric_limits<double>::max();
  }
  if (info.hasAttribute(QGeoPositionInfo::VerticalAccuracy)) {
    qreal accuracy = info.attribute(QGeoPositionInfo::VerticalAccuracy);
    if (!base::IsNaN(accuracy)) {
      // shield ourselves against invalid data
      position.altitude_accuracy = accuracy;
    }
  }
  if (info.hasAttribute(QGeoPositionInfo::GroundSpeed)) {
    qreal speed = info.attribute(QGeoPositionInfo::GroundSpeed);
    if (!base::IsNaN(speed)) {
      // shield ourselves against invalid data
      position.speed = speed;
    }
  }
  position.timestamp =
      base::Time::FromJsTime(info.timestamp().toMSecsSinceEpoch());
  return position;
}

class LocationSourceProxy
    : public QObject,
      public base::RefCountedThreadSafe<LocationSourceProxy> {
  Q_OBJECT

 public:
  LocationSourceProxy(LocationProvider* provider);

  bool HasSource() const;

  void StartUpdates() const;
  void StopUpdates() const;
  void RequestUpdate() const;

 private Q_SLOTS:
  void positionUpdated(const QGeoPositionInfo& info);
  void error(QGeoPositionInfoSource::Error error);

 private:
  friend class base::RefCountedThreadSafe<LocationSourceProxy>;

  virtual ~LocationSourceProxy();

  bool IsCurrentlyOnGeolocationThread() const;

  static bool IsCurrentlyOnIOThread();
  void InitializeOnIOThread();

  void SendNotifyPositionUpdated(const content::Geoposition& position);

  base::PlatformThreadId geolocation_thread_id_;
  scoped_refptr<base::SingleThreadTaskRunner> geolocation_thread_task_runner_;

  // It's only safe to access this on the geolocation thread
  base::WeakPtr<LocationProvider> provider_;

  // Should be accessed only on the IO thread
  QScopedPointer<QGeoPositionInfoSource, QScopedPointerDeleteLater> source_;

  bool initialized_on_io_thread_;

  DISALLOW_COPY_AND_ASSIGN(LocationSourceProxy);
};

void LocationSourceProxy::positionUpdated(const QGeoPositionInfo& info) {
  DCHECK(IsCurrentlyOnGeolocationThread());

  if (info.isValid()) {
    SendNotifyPositionUpdated(geopositionFromQt(info));
  } else {
    content::Geoposition error;
    error.error_code = content::Geoposition::ERROR_CODE_POSITION_UNAVAILABLE;
    SendNotifyPositionUpdated(error);
  }
}

void LocationSourceProxy::error(QGeoPositionInfoSource::Error error) {
  DCHECK(IsCurrentlyOnGeolocationThread());

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

  SendNotifyPositionUpdated(position);
}

LocationSourceProxy::~LocationSourceProxy() {
  disconnect(source_.data(), SIGNAL(positionUpdated(const QGeoPositionInfo&)),
             this, SLOT(positionUpdated(const QGeoPositionInfo&)));
  disconnect(source_.data(), SIGNAL(error(QGeoPositionInfoSource::Error)),
              this, SLOT(error(QGeoPositionInfoSource::Error)));
}

bool LocationSourceProxy::IsCurrentlyOnGeolocationThread() const {
  return geolocation_thread_id_ == base::PlatformThread::CurrentId();
}

// static
bool LocationSourceProxy::IsCurrentlyOnIOThread() {
  return content::BrowserThread::CurrentlyOn(content::BrowserThread::IO);
}

void LocationSourceProxy::InitializeOnIOThread() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

  initialized_on_io_thread_ = true;

  source_.reset(QGeoPositionInfoSource::createDefaultSource(NULL));
  if (source_) {
    connect(source_.data(), SIGNAL(positionUpdated(const QGeoPositionInfo&)),
            this, SLOT(positionUpdated(const QGeoPositionInfo&)));
    connect(source_.data(), SIGNAL(error(QGeoPositionInfoSource::Error)),
            this, SLOT(error(QGeoPositionInfoSource::Error)));
  }
}

void LocationSourceProxy::SendNotifyPositionUpdated(
    const content::Geoposition& position) {
  DCHECK(IsCurrentlyOnGeolocationThread());

  if (!provider_) {
    return;
  }

  provider_->NotifyPositionUpdated(position);
}

LocationSourceProxy::LocationSourceProxy(LocationProvider* provider)
    : geolocation_thread_id_(base::PlatformThread::CurrentId()),
      geolocation_thread_task_runner_(base::MessageLoopProxy::current()),
      provider_(provider->AsWeakPtr()),
      initialized_on_io_thread_(false) {
  qRegisterMetaType<QGeoPositionInfo>();
  qRegisterMetaType<QGeoPositionInfoSource::Error>();

  content::BrowserThread::PostTask(
      content::BrowserThread::IO,
      FROM_HERE,
      base::Bind(&LocationSourceProxy::InitializeOnIOThread, this));
}

bool LocationSourceProxy::HasSource() const {
  return true;
}

void LocationSourceProxy::StartUpdates() const {
  if (!IsCurrentlyOnIOThread()) {
    content::BrowserThread::PostTask(
        content::BrowserThread::IO,
        FROM_HERE,
        base::Bind(&LocationSourceProxy::StartUpdates, this));
    return;
  }

  DCHECK(initialized_on_io_thread_);

  if (source_) {
    source_->startUpdates();
  }
}

void LocationSourceProxy::StopUpdates() const {
  if (!IsCurrentlyOnIOThread()) {
    content::BrowserThread::PostTask(
        content::BrowserThread::IO,
        FROM_HERE,
        base::Bind(&LocationSourceProxy::StopUpdates, this));
    return;
  }

  DCHECK(initialized_on_io_thread_);

  if (source_) {
    source_->stopUpdates();
  }
}

void LocationSourceProxy::RequestUpdate() const {
  if (!IsCurrentlyOnIOThread()) {
    content::BrowserThread::PostTask(
        content::BrowserThread::IO,
        FROM_HERE,
        base::Bind(&LocationSourceProxy::RequestUpdate, this));
    return;
  }

  DCHECK(initialized_on_io_thread_);

  if (source_) {
    source_->requestUpdate();
  }
}

bool LocationProvider::StartProvider(bool high_accuracy) {
  DCHECK(CalledOnValidThread());

  Q_UNUSED(high_accuracy);
  if (running_) {
    return true;
  }

  if (!source_.get()) {
    source_ = new LocationSourceProxy(this);
  }

  if (!source_->HasSource()) {
    return false;
  }

  running_ = true;

  if (is_permission_granted_) {
    source_->StartUpdates();
  }

  return true;
}

void LocationProvider::StopProvider() {
  DCHECK(CalledOnValidThread());

  if (!running_) {
    return;
  }

  running_ = false;

  source_->StopUpdates();
}

void LocationProvider::GetPosition(content::Geoposition* position) {
  DCHECK(CalledOnValidThread());
  DCHECK(position);

  *position = position_;
}

void LocationProvider::RequestRefresh() {
  DCHECK(CalledOnValidThread());

  if (is_permission_granted_) {
    source_->RequestUpdate();
  }
}

void LocationProvider::OnPermissionGranted() {
  DCHECK(CalledOnValidThread());

  if (!is_permission_granted_) {
    is_permission_granted_ = true;
    if (running_) {
      source_->StartUpdates();
    }
  }
}

void LocationProvider::NotifyPositionUpdated(
    const content::Geoposition& position) {
  DCHECK(CalledOnValidThread());

  position_ = position;

  if (!running_) {
    return;
  }

  NotifyCallback(position_);
}

LocationProvider::LocationProvider() :
    running_(false),
    is_permission_granted_(false) {
  DCHECK(CalledOnValidThread());
}

LocationProvider::~LocationProvider() {
  DCHECK(CalledOnValidThread());
  StopProvider();
}

} // namespace qt
} // namespace oxide

#include "oxide_qt_location_provider.moc"
