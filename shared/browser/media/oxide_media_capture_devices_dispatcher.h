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

#ifndef _OXIDE_SHARED_BROWSER_MEDIA_MEDIA_CAPTURE_DEVICES_DISPATCHER_H_
#define _OXIDE_SHARED_BROWSER_MEDIA_MEDIA_CAPTURE_DEVICES_DISPATCHER_H_

#include "base/macros.h"
#include "base/observer_list.h"
#include "content/public/browser/media_observer.h"

template <typename T> struct DefaultSingletonTraits;

namespace content {
class BrowserContext;
struct MediaStreamDevice;
class MediaStreamDevices;
}

namespace oxide {

class MediaCaptureDevicesDispatcherObserver;

// I think the name of this class is a bit odd, but I couldn't think of a
// better one so I copied the name from Chrome
class MediaCaptureDevicesDispatcher : public content::MediaObserver {
 public:
  static MediaCaptureDevicesDispatcher* GetInstance();

  // Return the currently attached audio capture devices
  const content::MediaStreamDevices& GetAudioCaptureDevices();

  // Return the currently attached video capture devices
  const content::MediaStreamDevices& GetVideoCaptureDevices();

  // Returns the first audio capture device. This is used as a fallback
  // when the embedder doesn't specify a device
  const content::MediaStreamDevice* GetFirstAudioCaptureDevice();

  // Returns the first video capture device. This is used as a fallback
  // when the embedder doesn't specify a device
  const content::MediaStreamDevice* GetFirstVideoCaptureDevice();

  // Returns the audio capture device corresponding to the specified ID,
  // or nullptr if no device with that ID exists
  const content::MediaStreamDevice* FindAudioCaptureDeviceById(
      const std::string& id);

  // Returns the video capture device corresponding to the specified ID,
  // or nullptr if no device with that ID exists
  const content::MediaStreamDevice* FindVideoCaptureDeviceById(
      const std::string& id);

  // Get the default capture devices for the specified BrowserContext.
  // This delegates to MediaCaptureDevicesContext, and then falls back to
  // using the first devices
  bool GetDefaultCaptureDevicesForContext(
      content::BrowserContext* context,
      bool audio,
      bool video,
      content::MediaStreamDevices* devices);
                                        
 private:
  friend struct DefaultSingletonTraits<MediaCaptureDevicesDispatcher>;
  friend class MediaCaptureDevicesDispatcherObserver;

  MediaCaptureDevicesDispatcher();
  ~MediaCaptureDevicesDispatcher() override;

  void AddObserver(MediaCaptureDevicesDispatcherObserver* observer);
  void RemoveObserver(MediaCaptureDevicesDispatcherObserver* observer);

  void NotifyAudioCaptureDevicesChanged();
  void NotifyVideoCaptureDevicesChanged();

  // content::MediaObserver implementation
  void OnAudioCaptureDevicesChanged() override;
  void OnVideoCaptureDevicesChanged() override;
  void OnMediaRequestStateChanged(
      int render_process_id,
      int render_frame_id,
      int page_request_id,
      const GURL& security_origin,
      content::MediaStreamType stream_type,
      content::MediaRequestState state) override;
  void OnCreatingAudioStream(int render_process_id,
                             int render_frame_id) override;

  ObserverList<MediaCaptureDevicesDispatcherObserver> observers_;
  
  DISALLOW_COPY_AND_ASSIGN(MediaCaptureDevicesDispatcher);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_MEDIA_MEDIA_CAPTURE_DEVICES_DISPATCHER_H_
