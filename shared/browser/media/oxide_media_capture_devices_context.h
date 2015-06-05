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

#ifndef _OXIDE_SHARED_BROWSER_MEDIA_MEDIA_CAPTURE_DEVICES_CONTEXT_H_
#define _OXIDE_SHARED_BROWSER_MEDIA_MEDIA_CAPTURE_DEVICES_CONTEXT_H_

#include <string>

#include "base/macros.h"
#include "components/keyed_service/core/keyed_service.h"
#include "content/public/common/media_stream_request.h"

#include "shared/browser/media/oxide_media_capture_devices_dispatcher_observer.h"

namespace content {
class BrowserContext;
}

namespace oxide {

class MediaCaptureDevicesContextFactory;

class MediaCaptureDevicesContext : public KeyedService,
                                   public MediaCaptureDevicesDispatcherObserver {
 public:
  static MediaCaptureDevicesContext* Get(content::BrowserContext* context);

  std::string GetDefaultAudioDeviceId() const;
  bool SetDefaultAudioDeviceId(const std::string& id);

  std::string GetDefaultVideoDeviceId() const;
  bool SetDefaultVideoDeviceId(const std::string& id);

  const content::MediaStreamDevice* GetDefaultAudioDevice() const;
  const content::MediaStreamDevice* GetDefaultVideoDevice() const;

 private:
  friend class MediaCaptureDevicesContextFactory;

  MediaCaptureDevicesContext();
  ~MediaCaptureDevicesContext() override;

  // MediaCaptureDevicesDispatcherObserver implementation
  void OnAudioCaptureDevicesChanged() override;
  void OnVideoCaptureDevicesChanged() override;

  std::string default_audio_device_id_;
  std::string default_video_device_id_;

  DISALLOW_COPY_AND_ASSIGN(MediaCaptureDevicesContext);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_MEDIA_MEDIA_CAPTURE_DEVICES_CONTEXT_H_
