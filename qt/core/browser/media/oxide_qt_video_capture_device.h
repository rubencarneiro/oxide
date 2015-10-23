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

#ifndef _OXIDE_QT_CORE_BROWSER_MEDIA_VIDEO_CAPTURE_DEVICE_H_
#define _OXIDE_QT_CORE_BROWSER_MEDIA_VIDEO_CAPTURE_DEVICE_H_

#include <QCamera>
#include <QScopedPointer>

#include "media/capture/video/video_capture_device.h"

namespace oxide {
namespace qt {
class CameraFrameGrabber;

class VideoCaptureDevice : public media::VideoCaptureDevice {
 public:
  explicit VideoCaptureDevice(
      const media::VideoCaptureDevice::Name& device_name);
  explicit VideoCaptureDevice(QCamera::Position position);
  ~VideoCaptureDevice() override;

 private:
  // media::VideoCaptureDevice implementation
  void AllocateAndStart(
      const media::VideoCaptureParams& params,
      scoped_ptr<media::VideoCaptureDevice::Client> client) override;
  void StopAndDeAllocate() override;

  media::VideoCaptureDevice::Name device_name_;
  QCamera::Position position_;

  // These both live on the UI thread
  QScopedPointer<CameraFrameGrabber, QScopedPointerDeleteLater> view_finder_;
  QScopedPointer<QCamera, QScopedPointerDeleteLater> camera_;

  DISALLOW_COPY_AND_ASSIGN(VideoCaptureDevice);
};

}
}

#endif // _OXIDE_QT_CORE_BROWSER_MEDIA_VIDEO_CAPTURE_DEVICE_H_
