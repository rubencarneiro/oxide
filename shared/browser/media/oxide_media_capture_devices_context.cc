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

#include "oxide_media_capture_devices_context.h"

#include "base/memory/singleton.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

#include "shared/browser/oxide_browser_context.h"

#include "oxide_media_capture_devices_context_client.h"
#include "oxide_media_capture_devices_dispatcher.h"

namespace oxide {

class MediaCaptureDevicesContextFactory
    : public BrowserContextKeyedServiceFactory {
 public:
  static MediaCaptureDevicesContextFactory* GetInstance();
  static MediaCaptureDevicesContext* GetForContext(
      content::BrowserContext* context);

 private:
  friend struct base::DefaultSingletonTraits<MediaCaptureDevicesContextFactory>;

  MediaCaptureDevicesContextFactory();
  ~MediaCaptureDevicesContextFactory() override;

  // BrowserContextKeyedServiceFactory methods:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;

  DISALLOW_COPY_AND_ASSIGN(MediaCaptureDevicesContextFactory);
};

MediaCaptureDevicesContextFactory::MediaCaptureDevicesContextFactory()
    : BrowserContextKeyedServiceFactory(
        "MediaCaptureDevicesContext",
        BrowserContextDependencyManager::GetInstance()) {}

MediaCaptureDevicesContextFactory::~MediaCaptureDevicesContextFactory() {}

KeyedService* MediaCaptureDevicesContextFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  return new MediaCaptureDevicesContext();
}

content::BrowserContext*
MediaCaptureDevicesContextFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return BrowserContext::FromContent(context)->GetOriginalContext();
}

// static
MediaCaptureDevicesContextFactory*
MediaCaptureDevicesContextFactory::GetInstance() {
  return base::Singleton<MediaCaptureDevicesContextFactory>::get();
}

// static
MediaCaptureDevicesContext* MediaCaptureDevicesContextFactory::GetForContext(
    content::BrowserContext* context) {
  return static_cast<MediaCaptureDevicesContext*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

MediaCaptureDevicesContext::MediaCaptureDevicesContext()
    : client_(nullptr) {}

MediaCaptureDevicesContext::~MediaCaptureDevicesContext() {}

void MediaCaptureDevicesContext::OnAudioCaptureDevicesChanged() {
  if (default_audio_device_id_.empty()) {
    return;
  }

  const content::MediaStreamDevice* device =
      MediaCaptureDevicesDispatcher::GetInstance()
        ->FindAudioCaptureDeviceById(default_audio_device_id_);
  if (device) {
    return;
  }

  // The selected device has been removed
  default_audio_device_id_.clear();

  if (client_) {
    client_->DefaultAudioDeviceChanged();
  }
}

void MediaCaptureDevicesContext::OnVideoCaptureDevicesChanged() {
  if (default_video_device_id_.empty()) {
    return;
  }

  const content::MediaStreamDevice* device =
      MediaCaptureDevicesDispatcher::GetInstance()
        ->FindVideoCaptureDeviceById(default_video_device_id_);
  if (device) {
    return;
  }

  // The selected device has been removed
  default_video_device_id_.clear();

  if (client_) {
    client_->DefaultVideoDeviceChanged();
  }
}

// static
MediaCaptureDevicesContext* MediaCaptureDevicesContext::Get(
    content::BrowserContext* context) {
  return MediaCaptureDevicesContextFactory::GetForContext(context);
}

std::string MediaCaptureDevicesContext::GetDefaultAudioDeviceId() const {
  return default_audio_device_id_;
}

bool MediaCaptureDevicesContext::SetDefaultAudioDeviceId(
    const std::string& id) {
  if (!id.empty()) {
    const content::MediaStreamDevice* device =
        MediaCaptureDevicesDispatcher::GetInstance()
          ->FindAudioCaptureDeviceById(id);
    if (!device) {
      return false;
    }
  }

  default_audio_device_id_ = id;

  if (client_) {
    client_->DefaultAudioDeviceChanged();
  }

  return true;
}

std::string MediaCaptureDevicesContext::GetDefaultVideoDeviceId() const {
  return default_video_device_id_;
}

bool MediaCaptureDevicesContext::SetDefaultVideoDeviceId(
    const std::string& id) {
  if (!id.empty()) {
    const content::MediaStreamDevice* device =
        MediaCaptureDevicesDispatcher::GetInstance()
          ->FindVideoCaptureDeviceById(id);
    if (!device) {
      return false;
    }
  }

  default_video_device_id_ = id;

  if (client_) {
    client_->DefaultVideoDeviceChanged();
  }

  return true;
}

const content::MediaStreamDevice*
MediaCaptureDevicesContext::GetDefaultAudioDevice() const {
  return MediaCaptureDevicesDispatcher::GetInstance()
      ->FindAudioCaptureDeviceById(default_audio_device_id_);
}

const content::MediaStreamDevice*
MediaCaptureDevicesContext::GetDefaultVideoDevice() const {
  return MediaCaptureDevicesDispatcher::GetInstance()
      ->FindVideoCaptureDeviceById(default_video_device_id_);
}

} // namespace oxide
