// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include <QList>
#include <QMap>
#include <QString>

#include "qt/core/api/oxideqglobal.h"
#include "qt/core/api/oxideqmediacapturedevices.h"

#include "oxideqquickwebcontext.h"

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
      break;
    case OxideQVideoCaptureDevice::PositionBackFace:
      rv["position"] = "backface";
      break;
    default:
      rv["position"] = "unspecified";
  }
    
  return rv;
}

}

class OxideQQuickGlobalPrivate {
  Q_DECLARE_PUBLIC(OxideQQuickGlobal)
  Q_DISABLE_COPY(OxideQQuickGlobalPrivate)

 public:
  ~OxideQQuickGlobalPrivate();

 private:
  OxideQQuickGlobalPrivate(OxideQQuickGlobal* q);

  void availableAudioCaptureDevicesDidChange();
  void availableVideoCaptureDevicesDidChange();

  OxideQQuickGlobal* q_ptr;

  bool audio_capture_devices_need_update_;
  QList<QVariant> audio_capture_devices_;

  bool video_capture_devices_need_update_;
  QList<QVariant> video_capture_devices_;
};

OxideQQuickGlobalPrivate::OxideQQuickGlobalPrivate(OxideQQuickGlobal* q)
    : q_ptr(q),
      audio_capture_devices_need_update_(true),
      video_capture_devices_need_update_(true) {}

void OxideQQuickGlobalPrivate::availableAudioCaptureDevicesDidChange() {
  Q_Q(OxideQQuickGlobal);

  audio_capture_devices_need_update_ = true;
  Q_EMIT q->availableAudioCaptureDevicesChanged();
}

void OxideQQuickGlobalPrivate::availableVideoCaptureDevicesDidChange() {
  Q_Q(OxideQQuickGlobal);

  video_capture_devices_need_update_ = true;
  Q_EMIT q->availableVideoCaptureDevicesChanged();
}

OxideQQuickGlobalPrivate::~OxideQQuickGlobalPrivate() {}

/*!
\qmltype Oxide
\inqmlmodule com.canonical.Oxide 1.0

\brief Global object for Oxide functions

The \e{Oxide} object is a global object for accessing global functions exposed
by Oxide.
*/

OxideQQuickGlobal::OxideQQuickGlobal() :
    d_ptr(new OxideQQuickGlobalPrivate(this)) {
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

  connect(OxideQMediaCaptureDevices::instance(),
          SIGNAL(availableAudioDevicesChanged()),
          SLOT(availableAudioCaptureDevicesDidChange()));
  connect(OxideQMediaCaptureDevices::instance(),
          SIGNAL(availableVideoDevicesChanged()),
          SLOT(availableVideoCaptureDevicesDidChange()));
}

OxideQQuickGlobal::~OxideQQuickGlobal() {
  OxideQMediaCaptureDevices::instance()->disconnect(this);
}

/*!
\qmlproperty enumeration Oxide::processModel
\since OxideQt 1.4

The process model to use. Setting this determines whether Oxide will run web
content in sandboxed sub-processes or the application process. The default is
\e{Oxide.ProcessModelMultiProcess}.

This must be called before Oxide is started up (in your applications main(),
before using any other Oxide APIs). Calling it afterwards will have no effect.

Possible values are:

\value Oxide.ProcessModelMultiProcess
Multi-process mode. In this mode, web content runs in sandboxed sub-processes
rather than the application process. This mode provides the best level of
security and fault tolerance.

\value Oxide.ProcessModelSingleProcess
Single process mode. In this mode, web content runs in the application process.
Web content is not sandboxed, and crashes that would normally only affect a web
content process in multi-process mode will result in an application crash in
this mode.
*/

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

/*!
\qmlproperty int Oxide::maxRendererProcessCount
\since OxideQt 1.4

The maximum number of web content processes to run. Setting this to 0 will
reset it to the default, which is system dependent.

This is not a hard limit, as there are cases where web content processes will
not be shared (eg, web views in different web contexts, or incognito /
non-incognito web views).

This must be called before Oxide is started up (in your applications main(),
before using any other Oxide APIs). Calling it afterwards will have no effect.
*/

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

/*!
\qmlproperty WebContext Oxide::defaultWebContext

The global default WebContext. The default WebContext will be used for WebView's
that aren't created with an application supplied WebContext.

The default WebContext is also the only usable WebContext in single-process mode
(processModel is \e{Oxide.ProcessModelSingleProcess}.

Accessing this for the first time will create the default WebContext if it
hasn't been created already. Oxide retains ownership of the default WebContext -
applications mustn't delete it.
*/

OxideQQuickWebContext* OxideQQuickGlobal::defaultWebContext() {
  return OxideQQuickWebContext::defaultContext(true);
}

/*!
\qmlproperty list<variant> Oxide::availableAudioCaptureDevices
\since OxideQt 1.8

A list of audio capture devices detected by Oxide. Each item of the list is a
variant with the following properties:
\list
  \li id - The ID of this device. This ID is unique for this device and this
  session, but applications should not rely on this ID persisting between
  sessions.
  \li displayName - The display name of this device.
\endlist
*/

QVariant OxideQQuickGlobal::availableAudioCaptureDevices() {
  Q_D(OxideQQuickGlobal);

  if (!d->audio_capture_devices_need_update_) {
    return d->audio_capture_devices_;
  }

  d->audio_capture_devices_need_update_ = false;
  d->audio_capture_devices_.clear();

  QList<OxideQAudioCaptureDevice> devices =
      OxideQMediaCaptureDevices::instance()->availableAudioDevices();

  for (const auto& device : devices) {
    d->audio_capture_devices_ << AudioCaptureDeviceToVariant(device);
  }

  return d->audio_capture_devices_;
}

/*!
\qmlproperty list<variant> Oxide::availableVideoCaptureDevices
\since OxideQt 1.8

A list of video capture devices detected by Oxide. Each item of the list is a
variant with the following properties:
\list
  \li id - The ID of this device. This ID is unique for this device and this
  session, but applications should not rely on this ID persisting between
  sessions.
  \li displayName - The display name of this device.
  \li position - The position of this device. This can be one of "frontface",
  "backface" or "unspecified". On devices where this is not supported, this
  will be "unspecified"
\endlist
*/

QVariant OxideQQuickGlobal::availableVideoCaptureDevices() {
  Q_D(OxideQQuickGlobal);

  if (!d->video_capture_devices_need_update_) {
    return d->video_capture_devices_;
  }

  d->video_capture_devices_need_update_ = false;
  d->video_capture_devices_.clear();

  QList<OxideQVideoCaptureDevice> devices =
      OxideQMediaCaptureDevices::instance()->availableVideoDevices();

  for (const auto& device : devices) {
    d->video_capture_devices_ << VideoCaptureDeviceToVariant(device);
  }

  return d->video_capture_devices_;
}

/*!
\qmlproperty string Oxide::chromiumVersion
\since OxideQt 1.15

The Chromium version that this Oxide build is based on, in the form \e{x.x.x.x}.
*/

QString OxideQQuickGlobal::chromiumVersion() const {
  return oxideGetChromeVersion();
}

/*!
\qmlproperty string Oxide::oxideVersion
\since OxideQt 1.15

The current Oxide version, in the form \e{1.x.x}.
*/

QString OxideQQuickGlobal::oxideVersion() const {
  return oxideGetVersion();
}

#include "moc_oxideqquickglobal_p.cpp"
