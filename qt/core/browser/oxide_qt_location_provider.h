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

#include <QtGlobal>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "content/browser/geolocation/location_provider_base.h"
#include "content/public/common/geoposition.h"

QT_BEGIN_NAMESPACE
class QThread;
QT_END_NAMESPACE

namespace base {
class MessageLoopProxy;
}

namespace oxide {
namespace qt {

class LocationSource;

class LocationProvider FINAL : public content::LocationProviderBase {
 public:
  LocationProvider();
  ~LocationProvider();

  bool StartProvider(bool high_accuracy) FINAL;
  void StopProvider() FINAL;

  void GetPosition(content::Geoposition* position) FINAL;

  void RequestRefresh() FINAL;

  void OnPermissionGranted() FINAL;

 protected:
  friend class LocationSource;

  void cachePosition(const content::Geoposition& position);
  void notifyCallbackOnGeolocationThread(const content::Geoposition& position);
  void doNotifyCallback(const content::Geoposition& position);

 private:
  bool running_;
  scoped_refptr<base::MessageLoopProxy> proxy_;
  bool is_permission_granted_;
  LocationSource* source_;
  QThread* worker_thread_;
  content::Geoposition position_;

  void invokeOnWorkerThread(const char* method) const;

  DISALLOW_COPY_AND_ASSIGN(LocationProvider);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_LOCATION_PROVIDER_H_
