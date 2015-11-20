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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
// USA

#include "oxide_video_capture_device_hybris.h"

#include <hybris/camera/camera_compatibility_layer_capabilities.h>

#include "base/logging.h"
#include "base/single_thread_task_runner.h"
#include "base/thread_task_runner_handle.h"
#include "media/base/video_types.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_surface.h"

namespace oxide {

// static
void VideoCaptureDeviceHybris::OnMsgErrorCallback(void* context) {
  reinterpret_cast<VideoCaptureDeviceHybris*>(context)->OnError();
}

// static
void VideoCaptureDeviceHybris::OnDataRawImageCallback(void* data,
                                                      uint32_t size,
                                                      void* context) {
  reinterpret_cast<VideoCaptureDeviceHybris*>(context)
      ->OnFrameAvailable(data, size);
}

// static
void VideoCaptureDeviceHybris::OnPreviewTextureNeedsUpdateCallback(
    void* context) {
  reinterpret_cast<VideoCaptureDeviceHybris*>(context)
      ->OnPreviewTextureNeedsUpdate();
}

void VideoCaptureDeviceHybris::OnError() {
  // XXX: Can we get any more information about the error?
  client_->OnError(FROM_HERE, "Error from Hybris");
}

void VideoCaptureDeviceHybris::OnFrameAvailable(void* data, uint32_t size) {
  printf("OnFrameAvailable\n");
  client_->OnIncomingCapturedData(static_cast<uint8_t*>(data),
                                  size,
                                  capture_format_,
                                  0,
                                  base::TimeTicks::Now());
}

void DoUpdate(CameraControl* control) {
  android_camera_update_preview_texture(control);
}

void VideoCaptureDeviceHybris::OnPreviewTextureNeedsUpdate() {
  printf("OnPreviewTextureNeedsUpdate\n");
  // FIXME: THIS IS NOT SAFE (we don't ensure CameraControl stays alive).
  //  This is just here to get frame notifications from the camera
  task_runner_->PostTask(FROM_HERE,
                         base::Bind(&DoUpdate, camera_control_));
}

void VideoCaptureDeviceHybris::AllocateAndStart(
    const media::VideoCaptureParams& params,
    scoped_ptr<Client> client) {
  DCHECK(params.requested_format.IsValid());
  DCHECK(!camera_control_);

  client_ = client.Pass();

  task_runner_ = base::ThreadTaskRunnerHandle::Get();

  if (gfx::GetGLImplementation() != gfx::kGLImplementationEGLGLES2) {
    client_->OnError(FROM_HERE, "Unsupported GL implementation");
    return;
  }

  listener_.reset(new CameraControlListener());
  memset(listener_.get(), 0, sizeof(CameraControlListener));
  listener_->context = this;
  listener_->on_msg_error_cb = &OnMsgErrorCallback;
  listener_->on_data_raw_image_cb = &OnDataRawImageCallback;
  listener_->on_preview_texture_needs_update_cb =
      &OnPreviewTextureNeedsUpdateCallback;

  camera_control_ = android_camera_connect_to(type_, listener_.get());
  if (!camera_control_) {
    client_->OnError(FROM_HERE, "Couldn't create camera for specified type");
    return;
  }

  android_camera_set_preview_size(camera_control_,
                                  params.requested_format.frame_size.width(),
                                  params.requested_format.frame_size.height());
  android_camera_set_preview_fps(
      camera_control_,
      static_cast<int>(params.requested_format.frame_rate));
  // FIXME: Export this from libhybris
  //android_camera_set_preview_format(camera_control_,
  //                                  CAMERA_PIXEL_FORMAT_YUV420P);      

  // XXX: Not sure if we need to give the Hybris compat layer a texture. We're
  // not actually using it in Oxide
  gl_surface_ = gfx::GLSurface::CreateOffscreenGLSurface(gfx::Size(0, 0));
  gl_context_ = gfx::GLContext::CreateGLContext(nullptr,
                                                gl_surface_.get(),
                                                gfx::PreferIntegratedGpu);
  if (!gl_context_) {
    client_->OnError(FROM_HERE, "Failed to create GL context");
    return;
  }
  gl_context_->MakeCurrent(gl_surface_.get());
  glGenTextures(1, &preview_texture_);
  //glBindTexture(GL_TEXTURE_EXTERNAL_OES, preview_texture_);
  //glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_EXTERNAL_OES,
  //                GL_TEXTURE_WRAP_S,
  //                GL_CLAMP_TO_EDGE);
  //glTexParameteri(GL_TEXTURE_EXTERNAL_OES,
  //                GL_TEXTURE_WRAP_T,
  //                GL_CLAMP_TO_EDGE);

  android_camera_set_preview_texture(camera_control_, preview_texture_);

  int width = 0;
  int height = 0;
  android_camera_get_preview_size(camera_control_, &width, &height);

  int fps = 0;
  android_camera_get_preview_fps(camera_control_, &fps);

  capture_format_ =
      media::VideoCaptureFormat(gfx::Size(width, height),
                                fps,
                                media::PIXEL_FORMAT_I420,
                                media::PIXEL_STORAGE_CPU);

  android_camera_start_preview(camera_control_);
}

void VideoCaptureDeviceHybris::StopAndDeAllocate() {
  if (camera_control_) {
    android_camera_stop_preview(camera_control_);
    android_camera_disconnect(camera_control_);
    // FIXME: This deletes a reference counted object in the Hybris
    // compatibility layer, and results in a crash when creating a new
    // CameraControl
    //android_camera_delete(camera_control_);
    camera_control_ = nullptr;
  }

  gl_context_->MakeCurrent(gl_surface_.get());
  glDeleteTextures(1, &preview_texture_);

  gl_context_ = nullptr;
  gl_surface_ = nullptr;
}

VideoCaptureDeviceHybris::VideoCaptureDeviceHybris(CameraType type)
    : type_(type),
      camera_control_(nullptr) {}

VideoCaptureDeviceHybris::~VideoCaptureDeviceHybris() {
  StopAndDeAllocate();

  // XXX(chrisccoulson): The listener is called on another thread, and we're
  // about to delete it now. If Hybris doesn't provide a guarantee that the
  // listener will never be called once we're disconnected (and any in-progress
  // notification will have returned), then this is unsafe
}

} // namespace oxide
