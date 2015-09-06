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

#include <cmath>
#include <limits>

#include <QGeoCoordinate>
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#include <QMetaType>
#include <QMutex>
#include <QMutexLocker>
#include <QScopedPointer>
#include <QThread>
#include <QWaitCondition>

#include "base/bind.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/single_thread_task_runner.h"
#include "base/thread_task_runner_handle.h"
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
  if (std::isnan(position.altitude)) {
    // shield ourselves against invalid data
    position.altitude = 0;
  }
  if (info.hasAttribute(QGeoPositionInfo::HorizontalAccuracy)) {
    position.accuracy = info.attribute(QGeoPositionInfo::HorizontalAccuracy);
    if (std::isnan(position.accuracy)) {
      // shield ourselves against invalid data
      position.accuracy = std::numeric_limits<double>::max();
    }
  } else {
    // accuracy is mandatory
    position.accuracy = std::numeric_limits<double>::max();
  }
  if (info.hasAttribute(QGeoPositionInfo::VerticalAccuracy)) {
    qreal accuracy = info.attribute(QGeoPositionInfo::VerticalAccuracy);
    if (!std::isnan(accuracy)) {
      // shield ourselves against invalid data
      position.altitude_accuracy = accuracy;
    }
  }
  if (info.hasAttribute(QGeoPositionInfo::GroundSpeed)) {
    qreal speed = info.attribute(QGeoPositionInfo::GroundSpeed);
    if (!std::isnan(speed)) {
      // shield ourselves against invalid data
      position.speed = speed;
    }
  }
  position.timestamp =
      base::Time::FromJsTime(info.timestamp().toMSecsSinceEpoch());
  return position;
}

// LocationSourceProxy lives on the geolocation thread, and acts as a
// bridge between LocationProvider and the QGeoPositionInfoSource that
// lives on the IO thread. The reason we use the IO thread is because
// the geolocation thread doesn't have an IO message loop, so we can't use
// it for watching file descriptors with QSocketNotifier
class LocationSourceProxy
    : public QObject,
      public base::RefCountedThreadSafe<LocationSourceProxy> {
  Q_OBJECT

 public:
  static scoped_refptr<LocationSourceProxy> Create(LocationProvider* provider);

  void StartUpdates() const;
  void StopUpdates() const;
  void RequestUpdate() const;

 private Q_SLOTS:
  void positionUpdated(const QGeoPositionInfo& info);
  void error(QGeoPositionInfoSource::Error error);

 private:
  friend class base::RefCountedThreadSafe<LocationSourceProxy>;

  LocationSourceProxy(LocationProvider* provider);
  virtual ~LocationSourceProxy();

  bool Initialize();

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

  QMutex initialization_lock_;
  QWaitCondition initialization_waiter_;

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

LocationSourceProxy::LocationSourceProxy(LocationProvider* provider)
    : geolocation_thread_id_(base::PlatformThread::CurrentId()),
      geolocation_thread_task_runner_(base::ThreadTaskRunnerHandle::Get()),
      provider_(provider->AsWeakPtr()) {
  qRegisterMetaType<QGeoPositionInfo>();
  qRegisterMetaType<QGeoPositionInfoSource::Error>();
}

LocationSourceProxy::~LocationSourceProxy() {
  disconnect(source_.data(), SIGNAL(positionUpdated(const QGeoPositionInfo&)),
             this, SLOT(positionUpdated(const QGeoPositionInfo&)));
  disconnect(source_.data(), SIGNAL(error(QGeoPositionInfoSource::Error)),
              this, SLOT(error(QGeoPositionInfoSource::Error)));
}

bool LocationSourceProxy::Initialize() {
  DCHECK(IsCurrentlyOnGeolocationThread());

  QMutexLocker lock(&initialization_lock_);

  content::BrowserThread::PostTask(
      content::BrowserThread::IO,
      FROM_HERE,
      base::Bind(&LocationSourceProxy::InitializeOnIOThread, this));

  initialization_waiter_.wait(&initialization_lock_);

  return source_ != nullptr;
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

  source_.reset(QGeoPositionInfoSource::createDefaultSource(nullptr));

  {
    QMutexLocker lock(&initialization_lock_);
    initialization_waiter_.wakeAll();
  }

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

// static
scoped_refptr<LocationSourceProxy> LocationSourceProxy::Create(
    LocationProvider* provider) {
  // We have a separate initialize function because we need to have a
  // reference on this thread before posting the initialize task to the
  // IO thread, else the IO thread can drop its temporary reference
  // before we reference it - resulting in us returning an invalid pointer
  scoped_refptr<LocationSourceProxy> proxy(new LocationSourceProxy(provider));
  if (!proxy->Initialize()) {
    return scoped_refptr<LocationSourceProxy>();
  }

  return proxy;
}

void LocationSourceProxy::StartUpdates() const {
  if (!IsCurrentlyOnIOThread()) {
    content::BrowserThread::PostTask(
        content::BrowserThread::IO,
        FROM_HERE,
        base::Bind(&LocationSourceProxy::StartUpdates, this));
    return;
  }

  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

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

  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

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

  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

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
    source_ = LocationSourceProxy::Create(this);
  }

  if (!source_.get()) {
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

  if (is_permission_granted_ && running_) {
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
    is_permission_granted_(false) {}

LocationProvider::~LocationProvider() {
  DCHECK(CalledOnValidThread());
  StopProvider();
}

} // namespace qt
} // namespace oxide

#include "oxide_qt_location_provider.moc"
