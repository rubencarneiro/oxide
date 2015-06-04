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

#include "oxide_media_capture_devices_dispatcher.h"

#include "base/memory/singleton.h"

namespace oxide {

void MediaCaptureDevicesDispatcher::OnAudioCaptureDevicesChanged() {}

void MediaCaptureDevicesDispatcher::OnVideoCaptureDevicesChanged() {}

void MediaCaptureDevicesDispatcher::OnMediaRequestStateChanged(
    int render_process_id,
    int render_frame_id,
    int page_request_id,
    const GURL& security_origin,
    content::MediaStreamType stream_type,
    content::MediaRequestState state) {}

void MediaCaptureDevicesDispatcher::OnCreatingAudioStream(
    int render_process_id,
    int render_frame_id) {}

MediaCaptureDevicesDispatcher::MediaCaptureDevicesDispatcher() {}

// static
MediaCaptureDevicesDispatcher* MediaCaptureDevicesDispatcher::GetInstance() {
  return Singleton<MediaCaptureDevicesDispatcher>::get();
}

MediaCaptureDevicesDispatcher::~MediaCaptureDevicesDispatcher() {}

} // namespace oxide
