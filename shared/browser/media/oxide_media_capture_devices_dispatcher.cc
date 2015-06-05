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

#include "base/bind.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/memory/singleton.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/media_capture_devices.h"
#include "content/public/common/media_stream_request.h"

#include "oxide_media_capture_devices_context.h"
#include "oxide_media_capture_devices_dispatcher_observer.h"

namespace oxide {

namespace {

const content::MediaStreamDevice* GetFirstCaptureDevice(
    const content::MediaStreamDevices& devices) {
  if (devices.size() == 0) {
    return nullptr;
  }

  return &devices[0];
}

}

MediaCaptureDevicesDispatcher::MediaCaptureDevicesDispatcher() {}

MediaCaptureDevicesDispatcher::~MediaCaptureDevicesDispatcher() {}

void MediaCaptureDevicesDispatcher::AddObserver(
    MediaCaptureDevicesDispatcherObserver* observer) {
  observers_.AddObserver(observer);
}

void MediaCaptureDevicesDispatcher::RemoveObserver(
    MediaCaptureDevicesDispatcherObserver* observer) {
  observers_.RemoveObserver(observer);
}

void MediaCaptureDevicesDispatcher::NotifyAudioCaptureDevicesChanged() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  FOR_EACH_OBSERVER(MediaCaptureDevicesDispatcherObserver,
                    observers_,
                    OnAudioCaptureDevicesChanged());
}

void MediaCaptureDevicesDispatcher::NotifyVideoCaptureDevicesChanged() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  FOR_EACH_OBSERVER(MediaCaptureDevicesDispatcherObserver,
                    observers_,
                    OnVideoCaptureDevicesChanged());
}

void MediaCaptureDevicesDispatcher::OnAudioCaptureDevicesChanged() {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&MediaCaptureDevicesDispatcher::NotifyAudioCaptureDevicesChanged,
                 base::Unretained(this)));
}

void MediaCaptureDevicesDispatcher::OnVideoCaptureDevicesChanged() {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&MediaCaptureDevicesDispatcher::NotifyVideoCaptureDevicesChanged,
                 base::Unretained(this)));
}

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

// static
MediaCaptureDevicesDispatcher* MediaCaptureDevicesDispatcher::GetInstance() {
  return Singleton<MediaCaptureDevicesDispatcher>::get();
}

const content::MediaStreamDevices&
MediaCaptureDevicesDispatcher::GetAudioCaptureDevices() {
  return content::MediaCaptureDevices::GetInstance()->GetAudioCaptureDevices();
}

const content::MediaStreamDevices&
MediaCaptureDevicesDispatcher::GetVideoCaptureDevices() {
  return content::MediaCaptureDevices::GetInstance()->GetVideoCaptureDevices();
}

const content::MediaStreamDevice*
MediaCaptureDevicesDispatcher::GetFirstAudioCaptureDevice() {
  return GetFirstCaptureDevice(GetAudioCaptureDevices());
}

const content::MediaStreamDevice*
MediaCaptureDevicesDispatcher::GetFirstVideoCaptureDevice() {
  return GetFirstCaptureDevice(GetVideoCaptureDevices());
}

const content::MediaStreamDevice*
MediaCaptureDevicesDispatcher::FindAudioCaptureDeviceById(
    const std::string& id) {
  if (id.empty()) {
    return nullptr;
  }

  return GetAudioCaptureDevices().FindById(id);
}

const content::MediaStreamDevice*
MediaCaptureDevicesDispatcher::FindVideoCaptureDeviceById(
    const std::string& id) {
  if (id.empty()) {
    return nullptr;
  }

  return GetVideoCaptureDevices().FindById(id);
}

bool MediaCaptureDevicesDispatcher::GetDefaultCaptureDevicesForContext(
    content::BrowserContext* context,
    bool audio,
    bool video,
    content::MediaStreamDevices* devices) {
  bool need_audio = audio;
  bool need_video = video;

  MediaCaptureDevicesContext* dc = MediaCaptureDevicesContext::Get(context);

  if (need_audio) {
    const content::MediaStreamDevice* device = dc->GetDefaultAudioDevice();
    if (!device) {
      device = GetFirstAudioCaptureDevice();
    }
    if (device) {
      need_audio = false;
      devices->push_back(*device);
    }
  }

  if (need_video) {
    const content::MediaStreamDevice* device = dc->GetDefaultVideoDevice();
    if (!device) {
      device = GetFirstVideoCaptureDevice();
    }
    if (device) {
      need_video = false;
      devices->push_back(*device);
    }
  }

  return !(need_audio || need_video);
}

} // namespace oxide
