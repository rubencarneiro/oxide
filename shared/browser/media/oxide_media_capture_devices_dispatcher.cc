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
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/common/media_stream_request.h"

#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_browser_process_main.h"
#include "shared/browser/permissions/oxide_permission_request_dispatcher.h"
#include "shared/browser/permissions/oxide_permission_request_response.h"
#include "shared/browser/permissions/oxide_temporary_saved_permission_context.h"

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

const content::MediaStreamDevices& EmptyDevices() {
  static const content::MediaStreamDevices g_empty;
  return g_empty;
}

PermissionRequestResponse ToPermissionRequestResponse(
    TemporarySavedPermissionStatus status) {
  return status == TEMPORARY_SAVED_PERMISSION_STATUS_ALLOWED ?
      PERMISSION_REQUEST_RESPONSE_ALLOW :
      PERMISSION_REQUEST_RESPONSE_DENY;
}

TemporarySavedPermissionStatus ToTemporarySavedPermissionStatus(
    PermissionRequestResponse response) {
  DCHECK_NE(response, PERMISSION_REQUEST_RESPONSE_CANCEL);
  return response == PERMISSION_REQUEST_RESPONSE_ALLOW ?
      TEMPORARY_SAVED_PERMISSION_STATUS_ALLOWED :
      TEMPORARY_SAVED_PERMISSION_STATUS_DENIED;
}

void RespondToMediaAccessPermissionRequest(
    const content::MediaResponseCallback& callback,
    content::RenderFrameHost* render_frame_host,
    bool audio,
    bool video,
    const std::string& requested_audio_device_id,
    const std::string& requested_video_device_id,
    PermissionRequestResponse response) {
  if (response != PERMISSION_REQUEST_RESPONSE_ALLOW) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_PERMISSION_DENIED,
                 nullptr);
    return;
  }

  content::BrowserContext* context =
      render_frame_host->GetProcess()->GetBrowserContext();

  content::MediaStreamDevices devices;
  if (audio && !requested_audio_device_id.empty()) {
    const content::MediaStreamDevice* device =
        MediaCaptureDevicesDispatcher::GetInstance()->
            FindAudioCaptureDeviceById(requested_audio_device_id);
    if (device) {
      devices.push_back(*device);
      audio = false;
    }
  }

  if (video && !requested_video_device_id.empty()) {
    const content::MediaStreamDevice* device =
        MediaCaptureDevicesDispatcher::GetInstance()->
            FindVideoCaptureDeviceById(requested_video_device_id);
    if (device) {
      devices.push_back(*device);
      video = false;
    }
  }

  if (audio || video) {
      MediaCaptureDevicesDispatcher::GetInstance()
          ->GetDefaultCaptureDevicesForContext(context,
                                               audio,
                                               video,
                                               &devices);
  }

  callback.Run(devices,
               devices.empty() ?
                   content::MEDIA_DEVICE_NO_HARDWARE :
                   content::MEDIA_DEVICE_OK,
               nullptr);
}

void RespondToMediaAccessPermissionRequestCallback(
    const content::MediaResponseCallback& callback,
    int render_process_id,
    int render_frame_id,
    bool audio,
    bool video,
    const std::string& requested_audio_device_id,
    const std::string& requested_video_device_id,
    const GURL& requesting_origin,
    const GURL& embedding_origin,
    PermissionRequestResponse response) {
  content::RenderFrameHost* rfh =
      content::RenderFrameHost::FromID(render_process_id, render_frame_id);
  if (!rfh) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_PERMISSION_DENIED,
                 nullptr);
    return;
  }

  RespondToMediaAccessPermissionRequest(callback,
                                        rfh,
                                        audio,
                                        video,
                                        requested_audio_device_id,
                                        requested_video_device_id,
                                        response);

  if (response == PERMISSION_REQUEST_RESPONSE_CANCEL) {
    return;
  }

  BrowserContext* context =
      BrowserContext::FromContent(rfh->GetProcess()->GetBrowserContext());
  TemporarySavedPermissionContext* pc =
      context->GetTemporarySavedPermissionContext();

  if (audio) {
    pc->SetPermissionStatus(TEMPORARY_SAVED_PERMISSION_TYPE_MEDIA_DEVICE_MIC,
                            requesting_origin,
                            embedding_origin,
                            ToTemporarySavedPermissionStatus(response));
  }
  if (video) {
    pc->SetPermissionStatus(
        TEMPORARY_SAVED_PERMISSION_TYPE_MEDIA_DEVICE_CAMERA,
        requesting_origin,
        embedding_origin,
        ToTemporarySavedPermissionStatus(response));
  }
}

PermissionRequestCallback WrapMediaResponseCallback(
    const content::MediaResponseCallback& callback,
    int render_process_id,
    int render_frame_id,
    bool audio,
    bool video,
    const std::string& requested_audio_device_id,
    const std::string& requested_video_device_id,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
  return base::Bind(&RespondToMediaAccessPermissionRequestCallback,
                    callback,
                    render_process_id,
                    render_frame_id,
                    audio,
                    video,
                    requested_audio_device_id,
                    requested_video_device_id,
                    requesting_origin,
                    embedding_origin);
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
      base::Bind(
        &MediaCaptureDevicesDispatcher::NotifyAudioCaptureDevicesChanged,
        base::Unretained(this)));
}

void MediaCaptureDevicesDispatcher::OnVideoCaptureDevicesChanged() {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(
        &MediaCaptureDevicesDispatcher::NotifyVideoCaptureDevicesChanged,
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
  // We use LeakySingletonTraits here so that observers can be created
  // before Oxide has started up
  return base::Singleton<
      MediaCaptureDevicesDispatcher,
      base::LeakySingletonTraits<MediaCaptureDevicesDispatcher>>::get();
}

const content::MediaStreamDevices&
MediaCaptureDevicesDispatcher::GetAudioCaptureDevices() {
  // We might be created before Oxide startup
  if (!BrowserProcessMain::GetInstance()->IsRunning()) {
    return EmptyDevices();
  }

  return content::MediaCaptureDevices::GetInstance()->GetAudioCaptureDevices();
}

const content::MediaStreamDevices&
MediaCaptureDevicesDispatcher::GetVideoCaptureDevices() {
  // We might be created before Oxide startup
  if (!BrowserProcessMain::GetInstance()->IsRunning()) {
    return EmptyDevices();
  }

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
    const content::MediaStreamDevice* device =
        dc->GetDefaultAudioDevice();
    if (!device) {
      device = GetFirstAudioCaptureDevice();
    }
    if (device) {
      need_audio = false;
      devices->push_back(*device);
    }
  }

  if (need_video) {
    const content::MediaStreamDevice* device =
        dc->GetDefaultVideoDevice();
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

void MediaCaptureDevicesDispatcher::RequestMediaAccessPermission(
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback) {
  if (request.video_type == content::MEDIA_DEVICE_AUDIO_OUTPUT ||
      request.audio_type == content::MEDIA_DEVICE_AUDIO_OUTPUT) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_INVALID_STATE,
                 nullptr);
    return;
  }

  if (request.video_type == content::MEDIA_NO_SERVICE &&
      request.audio_type == content::MEDIA_NO_SERVICE) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_INVALID_STATE,
                 nullptr);
    return;
  }

  // Desktop / tab capture not supported
  if (request.video_type == content::MEDIA_DESKTOP_VIDEO_CAPTURE ||
      request.audio_type == content::MEDIA_DESKTOP_AUDIO_CAPTURE ||
      request.video_type == content::MEDIA_TAB_VIDEO_CAPTURE ||
      request.audio_type == content::MEDIA_TAB_AUDIO_CAPTURE) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_NOT_SUPPORTED,
                 nullptr);
    return;
  }

  // Only MEDIA_GENERATE_STREAM is valid here - MEDIA_DEVICE_ACCESS doesn't
  // come from media stream, MEDIA_ENUMERATE_DEVICES doesn't trigger a
  // permission request and MEDIA_OPEN_DEVICE is used from pepper
  if (request.request_type != content::MEDIA_GENERATE_STREAM) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_NOT_SUPPORTED,
                 nullptr);
    return;
  }

  if (request.audio_type == content::MEDIA_DEVICE_AUDIO_CAPTURE &&
      GetAudioCaptureDevices().empty()) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_NO_HARDWARE,
                 nullptr);
    return;
  }

  if (request.video_type == content::MEDIA_DEVICE_VIDEO_CAPTURE &&
      GetVideoCaptureDevices().empty()) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_NO_HARDWARE,
                 nullptr);
    return;
  }

  content::RenderFrameHost* rfh =
      content::RenderFrameHost::FromID(request.render_process_id,
                                       request.render_frame_id);
  if (!rfh) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_PERMISSION_DENIED,
                 nullptr);
    return;
  }

  content::WebContents* contents =
      content::WebContents::FromRenderFrameHost(rfh);
  if (!contents) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_PERMISSION_DENIED,
                 nullptr);
    return;
  }

  TemporarySavedPermissionStatus saved_status =
      TEMPORARY_SAVED_PERMISSION_STATUS_ALLOWED;

  BrowserContext* context =
      BrowserContext::FromContent(rfh->GetProcess()->GetBrowserContext());
  TemporarySavedPermissionContext* pc =
      context->GetTemporarySavedPermissionContext();

  bool audio = request.audio_type == content::MEDIA_DEVICE_AUDIO_CAPTURE;
  bool video = request.video_type == content::MEDIA_DEVICE_VIDEO_CAPTURE;

  GURL embedding_origin = contents->GetLastCommittedURL().GetOrigin();

  if (audio) {
    saved_status = pc->GetPermissionStatus(
        TEMPORARY_SAVED_PERMISSION_TYPE_MEDIA_DEVICE_MIC,
        request.security_origin,
        embedding_origin);
  }
  if (video && saved_status != TEMPORARY_SAVED_PERMISSION_STATUS_DENIED) {
    TemporarySavedPermissionStatus status = pc->GetPermissionStatus(
        TEMPORARY_SAVED_PERMISSION_TYPE_MEDIA_DEVICE_CAMERA,
        request.security_origin,
        embedding_origin);
    if (status == TEMPORARY_SAVED_PERMISSION_STATUS_DENIED ||
        saved_status == TEMPORARY_SAVED_PERMISSION_STATUS_ALLOWED) {
      saved_status = status;
    }
  }

  if (saved_status != TEMPORARY_SAVED_PERMISSION_STATUS_ASK) {
    RespondToMediaAccessPermissionRequest(
        callback,
        rfh,
        audio,
        video,
        request.requested_audio_device_id,
        request.requested_video_device_id,
        ToPermissionRequestResponse(saved_status));
    return;
  }

  PermissionRequestDispatcher::FromWebContents(contents)
      ->RequestMediaAccessPermission(
        rfh,
        request.security_origin,
        audio,
        video,
        WrapMediaResponseCallback(callback,
                                  request.render_process_id,
                                  request.render_frame_id,
                                  audio,
                                  video,
                                  request.requested_audio_device_id,
                                  request.requested_video_device_id,
                                  request.security_origin,
                                  embedding_origin));
}

} // namespace oxide
