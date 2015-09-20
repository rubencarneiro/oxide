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

#ifndef _OXIDE_QT_CORE_BROWSER_MEDIA_VIDEO_CAPTURE_DEVICE_FACTORY_H_
#define _OXIDE_QT_CORE_BROWSER_MEDIA_VIDEO_CAPTURE_DEVICE_FACTORY_H_

#include "media/base/video_capture_types.h"
#include "media/capture/video/video_capture_device_factory.h"

namespace oxide {
namespace qt {

class VideoCaptureDeviceFactory : public media::VideoCaptureDeviceFactory {
 public:
  explicit VideoCaptureDeviceFactory() = default;
  ~VideoCaptureDeviceFactory() override = default;

 private:
  // media::VideoCaptureDeviceFactory overrides
  scoped_ptr<media::VideoCaptureDevice> Create(
      const media::VideoCaptureDevice::Name& device_name) override;
  void GetDeviceNames(media::VideoCaptureDevice::Names* device_names) override;
  void GetDeviceSupportedFormats(
      const media::VideoCaptureDevice::Name& device,
      media::VideoCaptureFormats* supported_formats) override;

  DISALLOW_COPY_AND_ASSIGN(VideoCaptureDeviceFactory);
};

}
}

#endif // _OXIDE_QT_CORE_BROWSER_MEDIA_VIDEO_CAPTURE_DEVICE_FACTORY_H_
