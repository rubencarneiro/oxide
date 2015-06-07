// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_API_MEDIA_CAPTURE_DEVICES_P_H_
#define _OXIDE_QT_CORE_API_MEDIA_CAPTURE_DEVICES_P_H_

#include <QList>
#include <QtGlobal>

#include "base/macros.h"

#include "qt/core/api/oxideqmediacapturedevices.h"
#include "shared/browser/media/oxide_media_capture_devices_dispatcher_observer.h"

class OxideQMediaCaptureDevicesPrivate :
    public oxide::MediaCaptureDevicesDispatcherObserver {
  Q_DECLARE_PUBLIC(OxideQMediaCaptureDevices)

 public:
  ~OxideQMediaCaptureDevicesPrivate() override;

 private:
  OxideQMediaCaptureDevicesPrivate(OxideQMediaCaptureDevices* q);

  // oxide::MediaCaptureDevicesDispatcherObserver implementation
  void OnAudioCaptureDevicesChanged() override;
  void OnVideoCaptureDevicesChanged() override;

  OxideQMediaCaptureDevices* q_ptr;

  bool audio_devices_need_update_;
  QList<OxideQAudioCaptureDevice> audio_devices_;

  bool video_devices_need_update_;
  QList<OxideQVideoCaptureDevice> video_devices_;

  DISALLOW_COPY_AND_ASSIGN(OxideQMediaCaptureDevicesPrivate);
};

#endif // _OXIDE_QT_CORE_API_MEDIA_CAPTURE_DEVICES_P_H_
