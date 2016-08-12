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

#ifndef _OXIDE_QT_TESTS_MOCK_QPA_SCREEN_H_
#define _OXIDE_QT_TESTS_MOCK_QPA_SCREEN_H_

#include <QImage>
#include <QRect>
#include <Qt>
#include <QtGui/qpa/qplatformscreen.h>

class MockScreen : public QPlatformScreen {
 public:
  MockScreen(int id,
             const QRect& geometry,
             const QRect& work_area_in_screen,
             int depth,
             QImage::Format format,
             qreal dpr,
             int form_factor);
  ~MockScreen() override;

  int* form_factor() { return &form_factor_; }

  void setGeometry(const QRect& geometry,
                   const QRect& work_area_in_screen);
  void setOrientation(Qt::ScreenOrientation orientation);
  void setFormFactor(int form_factor);

 private:
  // QPlatformScreen implementation
  QRect geometry() const override;
  QRect availableGeometry() const override;
  int depth() const override;
  QImage::Format format() const override;
  qreal devicePixelRatio() const override;
  Qt::ScreenOrientation nativeOrientation() const override;
  Qt::ScreenOrientation orientation() const override;
  QList<QPlatformScreen*> virtualSiblings() const override;
  QString name() const override;

  int id_;

  // The screen's geometry in its native orientation in physical pixels
  QRect geometry_;

  // The work area in |geometry_|
  QRect work_area_in_screen_;

  int depth_;
  QImage::Format format_;
  qreal dpr_;

  int form_factor_;

  Qt::ScreenOrientation orientation_; // The current orientation
};

#endif // _OXIDE_QT_TESTS_MOCK_QPA_SCREEN_H_
