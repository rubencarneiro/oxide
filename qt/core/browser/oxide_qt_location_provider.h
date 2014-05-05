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

#ifndef _OXIDE_QT_CORE_BROWSER_LOCATION_PROVIDER_H_
#define _OXIDE_QT_CORE_BROWSER_LOCATION_PROVIDER_H_

#include "base/message_loop/message_loop.h"
#include "content/browser/geolocation/location_provider_base.h"

#include <QGeoPositionInfoSource>
#include <QMutex>
#include <QObject>
#include <QtGlobal>
#include <QThread>

QT_BEGIN_NAMESPACE
class QGeoPositionInfo;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class LocationWorkerThread;

class LocationProvider Q_DECL_FINAL : public content::LocationProviderBase {
 public:
  LocationProvider();
  ~LocationProvider();

  bool StartProvider(bool high_accuracy);
  void StopProvider();

  void GetPosition(content::Geoposition* position);

  void RequestRefresh();

  void OnPermissionGranted();

 protected:
  friend class LocationSource;

  void notifyCallbackOnGeolocationThread(const content::Geoposition& position);

 private:
  base::MessageLoop* message_loop_;
  bool is_permission_granted_;
  LocationWorkerThread* worker_;

  Q_DISABLE_COPY(LocationProvider)
};

class LocationSource Q_DECL_FINAL : public QObject {
  Q_OBJECT

 public:
  LocationSource(LocationProvider* provider, bool start);

  QGeoPositionInfo lastKnownPosition() const;
  void requestUpdate() const;

 private Q_SLOTS:
  void positionUpdated(const QGeoPositionInfo& info);
  void error(QGeoPositionInfoSource::Error error);

 private:
  LocationProvider* provider_;
  QGeoPositionInfoSource* source_;
};

class LocationWorkerThread Q_DECL_FINAL : public QThread {
  Q_OBJECT

 public:
  LocationWorkerThread(LocationProvider* provider, bool start);

  QGeoPositionInfo lastKnownPosition();
  void requestUpdate();

 protected:
  void run();

 private:
  LocationProvider* provider_;
  bool start_;
  QMutex mutex_;
  LocationSource* source_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_LOCATION_PROVIDER_H_
