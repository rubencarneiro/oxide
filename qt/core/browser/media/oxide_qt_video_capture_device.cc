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

#include "base/logging.h"
#include "base/strings/stringprintf.h"

namespace oxide {
namespace qt {

namespace {

media::VideoPixelFormat ToMediaVideoPixelFormat(
    QVideoFrame::PixelFormat format) {
  switch (format) {
    case QVideoFrame::Format_RGB32:
      return media::PIXEL_FORMAT_RGB32;
    case QVideoFrame::Format_ARGB32:
      return media::PIXEL_FORMAT_ARGB;
    case QVideoFrame::Format_RGB24:
      return media::PIXEL_FORMAT_RGB24;
    case QVideoFrame::Format_YUV420P:
      return media::PIXEL_FORMAT_I420;
    case QVideoFrame::Format_YV12:
      return media::PIXEL_FORMAT_YV12;
    case QVideoFrame::Format_UYVY:
      return media::PIXEL_FORMAT_UYVY;
    case QVideoFrame::Format_NV12:
      return media::PIXEL_FORMAT_NV12;
    case QVideoFrame::Format_NV21:
      return media::PIXEL_FORMAT_NV21;
    default:
      NOTREACHED();
      return media::PIXEL_FORMAT_UNKNOWN;
  }
}

}

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
  if (!isActive()) {
    setError(StoppedError);
    return false;
  }

  if (!frame.isValid()) {
    LOG(WARNING) << "Received an invalid video frame";
    setError(IncorrectFormatError);
    return false;
  }

  if (frame.handleType() != QAbstractVideoBuffer::NoHandle) {
    LOG(WARNING) <<
        "Received a video frame with unsupported handle type" <<
        frame.handleType();
    setError(IncorrectFormatError);
    return false;
  }

  media::VideoPixelFormat pixel_format =
      ToMediaVideoPixelFormat(frame.pixelFormat());
  if (pixel_format == media::PIXEL_FORMAT_UNKNOWN) {
    LOG(WARNING) <<
        "Received a video frame with unsupported pixel format: " <<
        frame.pixelFormat();
    setError(UnsupportedFormatError);
    return false;
  }

  QVideoFrame clone_frame(frame);
  clone_frame.map(QAbstractVideoBuffer::ReadOnly);

  media::VideoCaptureFormat format(gfx::Size(frame.width(),
                                             frame.height()),
                                   0, pixel_format);
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

  return QList<QVideoFrame::PixelFormat>()
      << QVideoFrame::Format_RGB32
      << QVideoFrame::Format_ARGB32
      << QVideoFrame::Format_RGB24
      << QVideoFrame::Format_YUV420P
      << QVideoFrame::Format_YV12
      << QVideoFrame::Format_UYVY
      << QVideoFrame::Format_NV12
      << QVideoFrame::Format_NV21;
}

void CameraFrameGrabber::cameraError(QCamera::Error error) {
  LOG(WARNING) << "Camera error " << error << ": " <<
               qPrintable(camera_->errorString());
  client_->OnError(base::StringPrintf(
      "Received error code %d from camera: %s",
      error, qPrintable(camera_->errorString())));
}

VideoCaptureDevice::VideoCaptureDevice(
    const media::VideoCaptureDevice::Name& device_name)
    : device_name_(device_name),
      position_(QCamera::UnspecifiedPosition) {}

VideoCaptureDevice::VideoCaptureDevice(QCamera::Position position)
    : position_(position) {}

VideoCaptureDevice::~VideoCaptureDevice() {}

void VideoCaptureDevice::AllocateAndStart(
    const media::VideoCaptureParams& params,
    scoped_ptr<media::VideoCaptureDevice::Client> client) {
  if (device_name_.id().empty()) {
    camera_.reset(new QCamera(position_));
  } else {
    camera_.reset(new QCamera(QByteArray(device_name_.id().c_str())));
  }
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
