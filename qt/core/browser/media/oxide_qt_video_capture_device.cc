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

#include "oxide_qt_video_capture_device.h"

#include <QAbstractVideoSurface>

namespace oxide {
namespace qt {

class CameraFrameGrabber: public QAbstractVideoSurface {
public:
  CameraFrameGrabber(scoped_ptr<media::VideoCaptureDevice::Client> client)
      : client_(client.Pass()) {
  }

  QList<QVideoFrame::PixelFormat> supportedPixelFormats(
      QAbstractVideoBuffer::HandleType) const {
    return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_RGB32;
  }

  bool present(const QVideoFrame &frame) {
    if (!frame.isValid()) {
      return false;
    }
    QVideoFrame clone_frame(frame);

    clone_frame.map(QAbstractVideoBuffer::ReadOnly);

    media::VideoCaptureFormat format(gfx::Size(frame.width(),
                                               frame.height()),
                                     0, media::PIXEL_FORMAT_RGB32);
    client_->OnIncomingCapturedData(
        clone_frame.bits(),
        clone_frame.height() * clone_frame.bytesPerLine(),
        format,
        0,
        base::TimeTicks::Now());

    clone_frame.unmap();
    return true;
  }


  scoped_ptr<media::VideoCaptureDevice::Client> client_;
};

VideoCaptureDevice::VideoCaptureDevice(
    const media::VideoCaptureDevice::Name& device_name)
    : camera_(QByteArray(device_name.name().c_str())) {
  camera_.moveToThread(&thread_);
}

VideoCaptureDevice::~VideoCaptureDevice() {
  thread_.wait();
}

void VideoCaptureDevice::AllocateAndStart(
    const media::VideoCaptureParams& params,
    scoped_ptr<VideoCaptureDevice::Client> client) {
  thread_.start();

  view_finder_.reset(new CameraFrameGrabber(client.Pass()));
  view_finder_->moveToThread(&thread_);
  camera_.setViewfinder(view_finder_.get());

  QMetaObject::invokeMethod(&camera_, "start", Qt::QueuedConnection);
}

void VideoCaptureDevice::StopAndDeAllocate() {
  QMetaObject::invokeMethod(&camera_, "stop", Qt::QueuedConnection);
  thread_.quit();
}

}
}
