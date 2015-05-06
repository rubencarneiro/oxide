// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#include "oxideqquickglobal_p.h"

#include <QMap>
#include <QString>

#include "qt/core/api/oxideqglobal.h"
#include "qt/core/api/oxideqmediacapturedevice.h"

#include "oxideqquickwebcontext_p.h"

namespace {

QVariant AudioCaptureDeviceToVariant(const OxideQAudioCaptureDevice& device) {
  QMap<QString, QVariant> rv;
  rv["id"] = device.id();
  rv["displayName"] = device.displayName();
  return rv;
}

QVariant VideoCaptureDeviceToVariant(const OxideQVideoCaptureDevice& device) {
  QMap<QString, QVariant> rv;
  rv["id"] = device.id();
  rv["displayName"] = device.displayName();

  switch (device.position()) {
    case OxideQVideoCaptureDevice::PositionFrontFace:
      rv["position"] = "frontface";
    case OxideQVideoCaptureDevice::PositionBackFace:
      rv["position"] = "backface";
    default:
      rv["position"] = "unspecified";
  }
    
  return rv;
}

}

class OxideQQuickGlobalPrivate {
 public:
  ~OxideQQuickGlobalPrivate() {}
  OxideQQuickGlobalPrivate() {}
};

OxideQQuickGlobal::OxideQQuickGlobal() :
    d_ptr(new OxideQQuickGlobalPrivate()) {
  Q_STATIC_ASSERT(
      ProcessModelMultiProcess ==
        static_cast<ProcessModel>(OxideProcessModelMultiProcess));
  Q_STATIC_ASSERT(
      ProcessModelSingleProcess ==
        static_cast<ProcessModel>(OxideProcessModelSingleProcess));
  Q_STATIC_ASSERT(
      ProcessModelProcessPerSiteInstance ==
        static_cast<ProcessModel>(OxideProcessModelProcessPerSiteInstance));
  Q_STATIC_ASSERT(
      ProcessModelProcessPerView ==
        static_cast<ProcessModel>(OxideProcessModelProcessPerView));
  Q_STATIC_ASSERT(
      ProcessModelProcessPerSite ==
        static_cast<ProcessModel>(OxideProcessModelProcessPerSite));
  Q_STATIC_ASSERT(
      ProcessModelSitePerProcess ==
        static_cast<ProcessModel>(OxideProcessModelSitePerProcess));
}

OxideQQuickGlobal::~OxideQQuickGlobal() {}

OxideQQuickGlobal::ProcessModel OxideQQuickGlobal::processModel() const {
  return static_cast<ProcessModel>(oxideGetProcessModel());
}

void OxideQQuickGlobal::setProcessModel(ProcessModel model) {
  if (model == processModel()) {
    return;
  }

  oxideSetProcessModel(static_cast<OxideProcessModel>(model));

  Q_EMIT processModelChanged();
}

int OxideQQuickGlobal::maxRendererProcessCount() const {
  return static_cast<int>(
      std::max(oxideGetMaxRendererProcessCount(),
               static_cast<size_t>(std::numeric_limits<int>::max())));
}

void OxideQQuickGlobal::setMaxRendererProcessCount(int count) {
  if (count < 0) {
    qWarning()
        << "Invalid maxRendererProcessCount "
        << "(must be > 0. Set to 0 to use the default maximum)";
    return;
  }

  if (count == maxRendererProcessCount()) {
    return;
  }

  oxideSetMaxRendererProcessCount(static_cast<size_t>(count));

  Q_EMIT maxRendererProcessCountChanged();
}

OxideQQuickWebContext* OxideQQuickGlobal::defaultWebContext() {
  return OxideQQuickWebContext::defaultContext(true);
}

QVariant OxideQQuickGlobal::availableAudioCaptureDevices() {
  QList<OxideQAudioCaptureDevice> devices =
      OxideQAudioCaptureDevice::availableDevices();

  QList<QVariant> rv;
  for (const OxideQAudioCaptureDevice& device : devices) {
    rv << AudioCaptureDeviceToVariant(device);
  }

  return rv;
}

QVariant OxideQQuickGlobal::availableVideoCaptureDevices() {
  QList<OxideQVideoCaptureDevice> devices =
      OxideQVideoCaptureDevice::availableDevices();

  QList<QVariant> rv;
  for (const OxideQVideoCaptureDevice& device : devices) {
    rv << VideoCaptureDeviceToVariant(device);
  }

  return rv;
}
