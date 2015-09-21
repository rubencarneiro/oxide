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

#include "base/logging.h"

#include "video_capture_device_factory_oxide.h"

namespace media {

namespace {
media::VideoCaptureDeviceFactoryFactory* gfactory;
}

// static
VideoCaptureDeviceFactory*
VideoCaptureDeviceFactory::CreateVideoCaptureDeviceFactory(
    scoped_refptr<base::SingleThreadTaskRunner> ui_task_runner) {
  DCHECK(gfactory);
  return gfactory();
}

const std::string VideoCaptureDevice::Name::GetModel() const {
  return "";
}

void SetVideoCaptureDeviceFactoryFactory(
    VideoCaptureDeviceFactoryFactory* factory) {
  gfactory = factory;
}

}
