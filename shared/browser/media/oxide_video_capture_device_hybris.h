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

#ifndef _OXIDE_SHARED_BROWSER_MEDIA_VIDEO_CAPTURE_DEVICE_HYBRIS_H_
#define _OXIDE_SHARED_BROWSER_MEDIA_VIDEO_CAPTURE_DEVICE_HYBRIS_H_

#include <hybris/camera/camera_compatibility_layer.h>

#include <memory>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "media/base/video_capture_types.h"
#include "media/capture/video/video_capture_device.h"

typedef unsigned int GLuint;

namespace base {
class SingleThreadTaskRunner;
}

namespace gl {
class GLContext;
class GLSurface;
}

namespace oxide {

class VideoCaptureDeviceHybris : public media::VideoCaptureDevice {
 public:
  VideoCaptureDeviceHybris(const Name& device_name);
  ~VideoCaptureDeviceHybris() override;

  static const char* GetDeviceIdPrefix();

 private:
  static void OnMsgErrorCallback(void* context);
  static void OnPreviewFrameCallback(void* data, uint32_t size, void* context);

  void OnError();
  void OnFrameAvailable(void* data, uint32_t size);

  // media::VideoCaptureDevice implementation
  void AllocateAndStart(const media::VideoCaptureParams& params,
                        std::unique_ptr<Client> client) override;
  void StopAndDeAllocate() override;

  Name device_name_;

  CameraType position_;
  int orientation_;

  std::unique_ptr<Client> client_;

  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  std::unique_ptr<CameraControlListener> listener_;

  media::VideoCaptureFormat capture_format_;

  scoped_refptr<gl::GLSurface> gl_surface_;
  scoped_refptr<gl::GLContext> gl_context_;
  GLuint preview_texture_;
  base::TimeTicks first_ref_time_;

  CameraControl* camera_control_;

  DISALLOW_COPY_AND_ASSIGN(VideoCaptureDeviceHybris);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_MEDIA_VIDEO_CAPTURE_DEVICE_HYBRIS_H_
