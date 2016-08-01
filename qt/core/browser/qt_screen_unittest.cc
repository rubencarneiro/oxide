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

#include <memory>
#include <vector>

#include <QCoreApplication>
#include <QCursor>
#include <QGuiApplication>
#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QRect>
#include <QScreen>
#include <QVariant>

#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/display/display.h"
#include "ui/gfx/geometry/rect.h"

#include "shared/test/mock_screen_observer.h"

#include "qt_screen.h"

using testing::_;

namespace oxide {
namespace qt {

class ScreenTest : public testing::Test {
 public:
  ScreenTest();
  ~ScreenTest() override;

 protected:
  void SetScreenGeometry(QScreen* screen,
                         const QRect& geometry,
                         const QRect& work_area_in_screen);
  void SetScreenOrientation(QScreen* screen,
                            Qt::ScreenOrientation orientation);

 private:
  // testing::Test implementation
  void SetUp();

  std::unique_ptr<Screen> screen_;

  QPointer<QObject> mock_qpa_shim_;
};

void ScreenTest::SetScreenGeometry(QScreen* screen,
                                   const QRect& geometry,
                                   const QRect& work_area_in_screen) {
  QMetaObject::invokeMethod(mock_qpa_shim_, "setScreenGeometry",
                            Qt::DirectConnection,
                            Q_ARG(QScreen*, screen),
                            Q_ARG(const QRect&, geometry),
                            Q_ARG(const QRect&, work_area_in_screen));
}

void ScreenTest::SetScreenOrientation(QScreen* screen,
                                      Qt::ScreenOrientation orientation) {
  QMetaObject::invokeMethod(mock_qpa_shim_, "setScreenOrientation",
                            Qt::DirectConnection,
                            Q_ARG(QScreen*, screen),
                            Q_ARG(Qt::ScreenOrientation, orientation));
}

void ScreenTest::SetUp() {
  QVariant v =
      QCoreApplication::instance()->property("_oxide_mock_qpa_shim_api");
  ASSERT_FALSE(v.isNull());

  mock_qpa_shim_ = v.value<QObject*>();
  ASSERT_NE(nullptr, mock_qpa_shim_);
}

ScreenTest::ScreenTest()
    : screen_(new Screen()) {}

ScreenTest::~ScreenTest() {
  if (mock_qpa_shim_) {
    QMetaObject::invokeMethod(mock_qpa_shim_, "resetScreens",
                              Qt::DirectConnection);
  }
}

TEST_F(ScreenTest, PrimaryDisplay) {
  display::Display d = oxide::Screen::GetInstance()->GetPrimaryDisplay();

  EXPECT_EQ(0, d.bounds().x());
  EXPECT_EQ(0, d.bounds().y());
  EXPECT_EQ(540, d.bounds().width());
  EXPECT_EQ(960, d.bounds().height());
  EXPECT_EQ(0, d.work_area().x());
  EXPECT_EQ(25, d.work_area().y());
  EXPECT_EQ(540, d.work_area().width());
  EXPECT_EQ(935, d.work_area().height());
  EXPECT_EQ(2, d.device_scale_factor());
  EXPECT_EQ(display::Display::ROTATE_0, d.rotation());
  EXPECT_TRUE(d.is_valid());
}

struct ScreenListTestRow {
  ScreenListTestRow(const char* name,
                    const gfx::Rect& bounds,
                    const gfx::Rect& work_area,
                    float device_scale_factor)
      : name(name),
        bounds(bounds),
        work_area(work_area),
        device_scale_factor(device_scale_factor) {}

  const char* name;
  gfx::Rect bounds;
  gfx::Rect work_area;
  float device_scale_factor;
};

class ScreenListTest
    : public ScreenTest,
      public testing::WithParamInterface<ScreenListTestRow> {};

INSTANTIATE_TEST_CASE_P(
    Displays,
    ScreenListTest,
    testing::Values(
        ScreenListTestRow("TEST0",
                          gfx::Rect(0, 0, 540, 960),
                          gfx::Rect(0, 25, 540, 935),
                          2),
        ScreenListTestRow("TEST1",
                          gfx::Rect(1080, 0, 1920, 1080),
                          gfx::Rect(1080, 25, 1920, 1055),
                          1),
        ScreenListTestRow("TEST2",
                          gfx::Rect(1500, 0, 1920, 1080),
                          gfx::Rect(1500, 25, 1920, 1055),
                          2)));
                           
TEST_P(ScreenListTest, AllDisplays) {
  const ScreenListTestRow& row = GetParam();

  QList<QScreen*> qs = QGuiApplication::screens();
  auto it = std::find_if(qs.begin(), qs.end(),
                         [&row](QScreen* screen) {
    return QString(row.name) == screen->name();
  });
  ASSERT_NE(qs.end(), it);

  display::Display display = Screen::GetInstance()->DisplayFromQScreen(*it);

  ASSERT_TRUE(display.is_valid());
  EXPECT_EQ(row.bounds.x(), display.bounds().x());
  EXPECT_EQ(row.bounds.y(), display.bounds().y());
  EXPECT_EQ(row.bounds.width(), display.bounds().width());
  EXPECT_EQ(row.bounds.height(), display.bounds().height());
  EXPECT_EQ(row.work_area.x(), display.work_area().x());
  EXPECT_EQ(row.work_area.y(), display.work_area().y());
  EXPECT_EQ(row.work_area.width(), display.work_area().width());
  EXPECT_EQ(row.work_area.height(), display.work_area().height());
  EXPECT_EQ(row.device_scale_factor, display.device_scale_factor());
  EXPECT_EQ(display::Display::ROTATE_0, display.rotation());
}

TEST_F(ScreenTest, CursorScreenPoint) {
  QPoint q = QCursor::pos();
  gfx::Point p = oxide::Screen::GetInstance()->GetCursorScreenPoint();

  EXPECT_EQ(q.x(), p.x());
  EXPECT_EQ(q.y(), p.y());
}

TEST_F(ScreenTest, GeometryChange) {
  MockScreenObserver obs;

  display::Display display = oxide::Screen::GetInstance()->GetPrimaryDisplay();
  display.set_bounds(gfx::Rect(0, 0, 1080, 1920));
  display.set_work_area(gfx::Rect(0, 50, 1080, 1870));

  EXPECT_CALL(obs, OnDisplayPropertiesChanged(DisplayEq(display))).Times(3);

  SetScreenGeometry(QGuiApplication::primaryScreen(),
                    QRect(0, 0, 2160, 3840),
                    QRect(0, 100, 2160, 3740));
}

struct ScreenOrientationTestRow {
  ScreenOrientationTestRow(
      const char* name,
      const std::array<Qt::ScreenOrientation, 4>& orientations,
      const std::array<display::Display::Rotation, 4>& expected,
      const std::array<gfx::Rect, 4>& bounds,
      const std::array<gfx::Rect, 4>& work_areas)
      : name(name),
        orientations(orientations),
        expected(expected),
        bounds(bounds),
        work_areas(work_areas) {}

  const char* name;
  std::array<Qt::ScreenOrientation, 4> orientations;
  std::array<display::Display::Rotation, 4> expected;
  std::array<gfx::Rect, 4> bounds;
  std::array<gfx::Rect, 4> work_areas;
};

class ScreenOrientationTest
    : public ScreenTest,
      public testing::WithParamInterface<ScreenOrientationTestRow> {};

INSTANTIATE_TEST_CASE_P(
    Orientations,
    ScreenOrientationTest,
    testing::Values(
        ScreenOrientationTestRow("TEST0",
                                 { Qt::LandscapeOrientation,
                                   Qt::InvertedPortraitOrientation,
                                   Qt::InvertedLandscapeOrientation,
                                   Qt::PortraitOrientation },
                                 { display::Display::ROTATE_270,
                                   display::Display::ROTATE_180,
                                   display::Display::ROTATE_90,
                                   display::Display::ROTATE_0 },
                                 { gfx::Rect(0, 0, 960, 540),
                                   gfx::Rect(0, 0, 540, 960),
                                   gfx::Rect(0, 0, 960, 540),
                                   gfx::Rect(0, 0, 540, 960) },
                                 { gfx::Rect(0, 25, 960, 515),
                                   gfx::Rect(0, 25, 540, 935),
                                   gfx::Rect(0, 25, 960, 515),
                                   gfx::Rect(0, 25, 540, 935) }),
        ScreenOrientationTestRow("TEST1",
                                 { Qt::InvertedPortraitOrientation,
                                   Qt::InvertedLandscapeOrientation,
                                   Qt::PortraitOrientation,
                                   Qt::LandscapeOrientation },
                                 { display::Display::ROTATE_270,
                                   display::Display::ROTATE_180,
                                   display::Display::ROTATE_90,
                                   display::Display::ROTATE_0 },
                                 { gfx::Rect(1080, 0, 1080, 1920),
                                   gfx::Rect(1080, 0, 1920, 1080),
                                   gfx::Rect(1080, 0, 1080, 1920),
                                   gfx::Rect(1080, 0, 1920, 1080) },
                                 { gfx::Rect(1080, 25, 1080, 1895),
                                   gfx::Rect(1080, 25, 1920, 1055),
                                   gfx::Rect(1080, 25, 1080, 1895),
                                   gfx::Rect(1080, 25, 1920, 1055) })));

TEST_P(ScreenOrientationTest, OrientationChange) {
  MockScreenObserver obs;

  const ScreenOrientationTestRow& row = GetParam();

  QList<QScreen*> q_screens = QGuiApplication::screens();
  auto it = std::find_if(q_screens.begin(), q_screens.end(),
                         [&row](QScreen* screen) {
    return QString(row.name) == screen->name();
  });
  ASSERT_NE(q_screens.end(), it);

  QScreen* q_screen = *it;
  display::Display display = Screen::GetInstance()->DisplayFromQScreen(q_screen);

  {
    testing::InSequence dummy;

    display.set_bounds(row.bounds[0]);
    display.set_work_area(row.work_areas[0]);
    EXPECT_CALL(obs, OnDisplayPropertiesChanged(DisplayEq(display))).Times(3);
    display.set_rotation(row.expected[0]);
    EXPECT_CALL(obs, OnDisplayPropertiesChanged(DisplayEq(display)));

    display.set_bounds(row.bounds[1]);
    display.set_work_area(row.work_areas[1]);
    EXPECT_CALL(obs, OnDisplayPropertiesChanged(DisplayEq(display))).Times(3);
    display.set_rotation(row.expected[1]);
    EXPECT_CALL(obs, OnDisplayPropertiesChanged(DisplayEq(display)));

    display.set_bounds(row.bounds[2]);
    display.set_work_area(row.work_areas[2]);
    EXPECT_CALL(obs, OnDisplayPropertiesChanged(DisplayEq(display))).Times(3);
    display.set_rotation(row.expected[2]);
    EXPECT_CALL(obs, OnDisplayPropertiesChanged(DisplayEq(display)));

    display.set_bounds(row.bounds[3]);
    display.set_work_area(row.work_areas[3]);
    EXPECT_CALL(obs, OnDisplayPropertiesChanged(DisplayEq(display))).Times(3);
    display.set_rotation(row.expected[3]);
    EXPECT_CALL(obs, OnDisplayPropertiesChanged(DisplayEq(display)));
  }

  SetScreenOrientation(q_screen, row.orientations[0]);
  EXPECT_EQ(row.expected[0],
            Screen::GetInstance()->DisplayFromQScreen(q_screen).rotation());

  SetScreenOrientation(q_screen, row.orientations[1]);
  EXPECT_EQ(row.expected[1],
            Screen::GetInstance()->DisplayFromQScreen(q_screen).rotation());

  SetScreenOrientation(q_screen, row.orientations[2]);
  EXPECT_EQ(row.expected[2],
            Screen::GetInstance()->DisplayFromQScreen(q_screen).rotation());

  SetScreenOrientation(q_screen, row.orientations[3]);
  EXPECT_EQ(row.expected[3],
            Screen::GetInstance()->DisplayFromQScreen(q_screen).rotation());
}

// Calling Screen::DisplayFromQScreen with a null screen shouldn't leave a
// ghost display in the display list
TEST_F(ScreenTest, NullQScreen) {
  EXPECT_EQ(3U, Screen::GetInstance()->GetAllDisplays().size());

  display::Display display = Screen::GetInstance()->DisplayFromQScreen(nullptr);
  EXPECT_FALSE(display.is_valid());

  EXPECT_EQ(3U, Screen::GetInstance()->GetAllDisplays().size());
}

} // namespace qt
} // namespace oxide
