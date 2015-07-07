// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#ifndef OXIDE_MEDIA_VIDEO_CAPTURE_LINUX_VIDEO_CAPTURE_DEVICE_LINUX_H_
#define OXIDE_MEDIA_VIDEO_CAPTURE_LINUX_VIDEO_CAPTURE_DEVICE_LINUX_H_

#include "media/video/capture/video_capture_device.h"

#include <QCamera>
#include <QThread>

namespace oxide {
namespace qt {
class CameraFrameGrabber;

class VideoCaptureDevice : public media::VideoCaptureDevice {
 public:
  explicit VideoCaptureDevice(const media::VideoCaptureDevice::Name& device_name);
  ~VideoCaptureDevice() override;

  void AllocateAndStart(const media::VideoCaptureParams& params,
                        scoped_ptr<Client> client) override;
  void StopAndDeAllocate() override;
 private:
  QCamera camera_;
  QThread thread_;
  scoped_ptr<CameraFrameGrabber> view_finder_;

  DISALLOW_COPY_AND_ASSIGN(VideoCaptureDevice);
};

}
}

#endif
