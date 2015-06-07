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

#include "../oxideqmediacapturedevices.h"
#include "../oxideqmediacapturedevices_p.h"

OxideQMediaCaptureDevicesPrivate::OxideQMediaCaptureDevicesPrivate(
    OxideQMediaCaptureDevices* q)
    : q_ptr(q),
      audio_devices_need_update_(true),
      video_devices_need_update_(true) {}

void OxideQMediaCaptureDevicesPrivate::OnAudioCaptureDevicesChanged() {
  Q_Q(OxideQMediaCaptureDevices);

  audio_devices_need_update_ = true;
  Q_EMIT q->availableAudioDevicesChanged();
}

void OxideQMediaCaptureDevicesPrivate::OnVideoCaptureDevicesChanged() {
  Q_Q(OxideQMediaCaptureDevices);

  video_devices_need_update_ = true;
  Q_EMIT q->availableVideoDevicesChanged();
}

OxideQMediaCaptureDevicesPrivate::~OxideQMediaCaptureDevicesPrivate() {}
