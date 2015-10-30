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

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "media/capture/video/video_capture_device_factory.h"

namespace oxide {

typedef base::Callback<void(scoped_ptr<media::VideoCaptureDevice::Names>)>
    EnumerateDevicesCallback;

class VideoCaptureDeviceFactoryLinux
    : public media::VideoCaptureDeviceFactory {
 public:
  VideoCaptureDeviceFactoryLinux(
      scoped_ptr<media::VideoCaptureDeviceFactory> delegate);
  ~VideoCaptureDeviceFactoryLinux() override;

 private:
  // media::VideoCaptureDeviceFactory implementation
  scoped_ptr<media::VideoCaptureDevice> Create(
      const media::VideoCaptureDevice::Name& device_name) override;
  void EnumerateDeviceNames(const EnumerateDevicesCallback& callback) override;
  void GetDeviceSupportedFormats(
      const media::VideoCaptureDevice::Name& device,
      media::VideoCaptureFormats* supported_formats) override;
  void GetDeviceNames(media::VideoCaptureDevice::Names* device_names) override;

  scoped_ptr<media::VideoCaptureDeviceFactory> delegate_;

  DISALLOW_COPY_AND_ASSIGN(VideoCaptureDeviceFactoryLinux);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_MEDIA_VIDEO_CAPTURE_DEVICE_FACTORY_LINUX_H_
