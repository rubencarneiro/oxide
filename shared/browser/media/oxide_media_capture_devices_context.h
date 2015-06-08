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

class MediaCaptureDevicesContextClient;
class MediaCaptureDevicesContextFactory;

// This class provides per-context default settings for capture devices.
// It's shared between each pair of BrowserContexts
class MediaCaptureDevicesContext : public KeyedService,
                                   public MediaCaptureDevicesDispatcherObserver {
 public:
  static MediaCaptureDevicesContext* Get(content::BrowserContext* context);

  void set_client(MediaCaptureDevicesContextClient* client) {
    client_ = client;
  }

  // Return the ID for the default audio capture device. The ID will be
  // empty if no default is provided
  std::string GetDefaultAudioDeviceId() const;

  // Set the ID for the default audio capture device. If the specified ID
  // is empty, the first audio device will be used for capture. If it isn't
  // empty, the ID must correspond to a valid device. Invalid device IDs are
  // ignored.
  // If the specified device is removed from the system in the future, the
  // ID will reset to empty and the client will be notified.
  // Returns true on success or false if the ID is not a valid device
  bool SetDefaultAudioDeviceId(const std::string& id);

  // Return the ID for the default video capture device. The ID will be
  // empty if no default is provided
  std::string GetDefaultVideoDeviceId() const;

  // Set the ID for the default video capture device. If the specified ID
  // is empty, the first video device will be used for capture. If it isn't
  // empty, the ID must correspond to a valid device. Invalid device IDs are
  // ignored.
  // If the specified device is removed from the system in the future, the
  // ID will reset to empty and the client will be notified.
  // Returns true on success or false if the ID is not a valid device
  bool SetDefaultVideoDeviceId(const std::string& id);

  // Returns the default audio capture device for this context, or nullptr
  // if none is selected
  const content::MediaStreamDevice* GetDefaultAudioDevice() const;

  // Returns the default video capture device for this context, or nullptr
  // if none is selected
  const content::MediaStreamDevice* GetDefaultVideoDevice() const;

 private:
  friend class MediaCaptureDevicesContextFactory;

  MediaCaptureDevicesContext();
  ~MediaCaptureDevicesContext() override;

  // MediaCaptureDevicesDispatcherObserver implementation
  void OnAudioCaptureDevicesChanged() override;
  void OnVideoCaptureDevicesChanged() override;

  MediaCaptureDevicesContextClient* client_;

  std::string default_audio_device_id_;
  std::string default_video_device_id_;

  DISALLOW_COPY_AND_ASSIGN(MediaCaptureDevicesContext);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_MEDIA_MEDIA_CAPTURE_DEVICES_CONTEXT_H_
