// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#include "screen.h"

#include <QtDebug>
#include <QtGui/qpa/qwindowsysteminterface.h>

QRect MockScreen::geometry() const {
  if (!override_geometry_.isNull()) {
    return override_geometry_;
  }
  return geometry_;
}

QRect MockScreen::availableGeometry() const {
  if (!override_available_geometry_.isNull()) {
    return override_available_geometry_;
  }
  return available_geometry_;
}

int MockScreen::depth() const {
  return depth_;
}

QImage::Format MockScreen::format() const {
  return format_;
}

qreal MockScreen::devicePixelRatio() const {
  return dpr_;
}

Qt::ScreenOrientation MockScreen::nativeOrientation() const {
  return native_orientation_;
}

Qt::ScreenOrientation MockScreen::orientation() const {
  return orientation_;
}

MockScreen::MockScreen(const QRect& geometry,
                       const QRect& available_geometry,
                       int depth,
                       QImage::Format format,
                       qreal dpr)
    : geometry_(geometry),
      available_geometry_(available_geometry),
      depth_(depth),
      format_(format),
      dpr_(dpr) {
  native_orientation_ = orientation_ = geometry.width() > geometry.height() ?
      Qt::LandscapeOrientation : Qt::PortraitOrientation;
}

MockScreen::~MockScreen() = default;

void MockScreen::overrideGeometry(const QRect& geometry,
                                  const QRect& available_geometry) {
  override_geometry_ = geometry;
  override_available_geometry_ = available_geometry;
  QWindowSystemInterface::handleScreenGeometryChange(screen(),
                                                     this->geometry(),
                                                     availableGeometry());
}

void MockScreen::setOrientation(Qt::ScreenOrientation orientation) {
  orientation_ = orientation;
  QWindowSystemInterface::handleScreenOrientationChange(screen(),
                                                        this->orientation());
}

void MockScreen::reset() {
  override_geometry_ = QRect();
  override_available_geometry_ = QRect();
  orientation_ = native_orientation_;
  QWindowSystemInterface::handleScreenGeometryChange(screen(),
                                                     geometry(),
                                                     availableGeometry());
  QWindowSystemInterface::handleScreenOrientationChange(screen(),
                                                        orientation());
}
