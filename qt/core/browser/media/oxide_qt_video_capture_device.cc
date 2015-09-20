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
#include <QCoreApplication>
#include <QPointer>
#include <QtGlobal>

#include "base/strings/stringprintf.h"

namespace oxide {
namespace qt {

class CameraFrameGrabber: public QAbstractVideoSurface {
  Q_OBJECT

 public:
  CameraFrameGrabber(scoped_ptr<media::VideoCaptureDevice::Client> client,
                     QCamera* camera)
      : client_(client.Pass()),
        camera_(camera) {}

 private:
  // QAbstractVideoSurface implementation
  bool present(const QVideoFrame &frame) override;
  bool start(const QVideoSurfaceFormat& format) override;
  QList<QVideoFrame::PixelFormat> supportedPixelFormats(
      QAbstractVideoBuffer::HandleType type) const override;

 private Q_SLOTS:
  void cameraError(QCamera::Error error);

 private:
  scoped_ptr<media::VideoCaptureDevice::Client> client_;
  QPointer<QCamera> camera_;
};

bool CameraFrameGrabber::present(const QVideoFrame& frame) {
  if (!frame.isValid()) {
    return false;
  }

  if (frame.handleType() != QAbstractVideoBuffer::NoHandle) {
    return false;
  }

  if (frame.pixelFormat() != QVideoFrame::Format_RGB32) {
    return false;
  }

  QVideoFrame clone_frame(frame);
  clone_frame.map(QAbstractVideoBuffer::ReadOnly);

  media::VideoCaptureFormat format(gfx::Size(frame.width(),
                                             frame.height()),
                                   0, media::PIXEL_FORMAT_RGB32);
  client_->OnIncomingCapturedData(clone_frame.bits(),
                                  clone_frame.mappedBytes(),
                                  format,
                                  0,
                                  base::TimeTicks::Now());

  clone_frame.unmap();
  return true;
}

bool CameraFrameGrabber::start(const QVideoSurfaceFormat& format) {
  if (!isFormatSupported(format)) {
    return false;
  }

  return QAbstractVideoSurface::start(format);
}

QList<QVideoFrame::PixelFormat> CameraFrameGrabber::supportedPixelFormats(
    QAbstractVideoBuffer::HandleType type) const {
  if (type != QAbstractVideoBuffer::NoHandle) {
    return QList<QVideoFrame::PixelFormat>();
  }

  return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_RGB32;
  // XXX: Support:
  //  Format_ARGB32
  //  Format_RGB24
  //  Format_YUV420P
  //  Format_YV12
  //  Format_UYVY
  //  Format_NV12
  //  Format_NV21
}

void CameraFrameGrabber::cameraError(QCamera::Error error) {
  client_->OnError(base::StringPrintf(
      "Received error code %d from camera: %s",
      error, qUtf8Printable(camera_->errorString())));
}

VideoCaptureDevice::VideoCaptureDevice(
    const media::VideoCaptureDevice::Name& device_name)
    : device_name_(device_name) {}

VideoCaptureDevice::~VideoCaptureDevice() {}

void VideoCaptureDevice::AllocateAndStart(
    const media::VideoCaptureParams& params,
    scoped_ptr<media::VideoCaptureDevice::Client> client) {
  camera_.reset(new QCamera(QByteArray(device_name_.name().c_str())));
  camera_->moveToThread(QCoreApplication::instance()->thread());
  camera_->setCaptureMode(QCamera::CaptureVideo);

  view_finder_.reset(new CameraFrameGrabber(client.Pass(), camera_.data()));
  view_finder_->moveToThread(QCoreApplication::instance()->thread());

  camera_->setViewfinder(view_finder_.data());
  view_finder_->connect(camera_.data(), SIGNAL(error(QCamera::Error)),
                        SLOT(cameraError(QCamera::Error)));

  QMetaObject::invokeMethod(camera_.data(), "start", Qt::QueuedConnection);
}

void VideoCaptureDevice::StopAndDeAllocate() {
  QMetaObject::invokeMethod(camera_.data(), "stop", Qt::QueuedConnection);

  camera_.reset();
  view_finder_.reset();
}

}
}

#include "oxide_qt_video_capture_device.moc"
