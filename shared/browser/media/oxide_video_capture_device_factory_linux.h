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

#ifndef _OXIDE_SHARED_BROWSER_MEDIA_VIDEO_CAPTURE_DEVICE_FACTORY_LINUX_H_
#define _OXIDE_SHARED_BROWSER_MEDIA_VIDEO_CAPTURE_DEVICE_FACTORY_LINUX_H_

#include <memory>

#include "base/macros.h"
#include "media/capture/video/video_capture_device_descriptor.h"
#include "media/capture/video/video_capture_device_factory.h"

namespace oxide {

typedef base::Callback<void(std::unique_ptr<media::VideoCaptureDeviceDescriptors>)>
    EnumerateDevicesCallback;

class VideoCaptureDeviceFactoryLinux
    : public media::VideoCaptureDeviceFactory {
 public:
  VideoCaptureDeviceFactoryLinux(
      std::unique_ptr<media::VideoCaptureDeviceFactory> delegate);
  ~VideoCaptureDeviceFactoryLinux() override;

 private:
  // media::VideoCaptureDeviceFactory implementation
  std::unique_ptr<media::VideoCaptureDevice> CreateDevice(
      const media::VideoCaptureDeviceDescriptor& device_descriptor) override;
  void EnumerateDeviceDescriptors(
      const EnumerateDevicesCallback& callback) override;
  void GetSupportedFormats(
      const media::VideoCaptureDeviceDescriptor& device_descriptor,
      media::VideoCaptureFormats* supported_formats) override;
  void GetDeviceDescriptors(
      media::VideoCaptureDeviceDescriptors* device_descriptors) override;

  std::unique_ptr<media::VideoCaptureDeviceFactory> platform_factory_;

  DISALLOW_COPY_AND_ASSIGN(VideoCaptureDeviceFactoryLinux);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_MEDIA_VIDEO_CAPTURE_DEVICE_FACTORY_LINUX_H_
