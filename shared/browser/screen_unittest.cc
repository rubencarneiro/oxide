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

#include <vector>

#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/display/display.h"
#include "ui/gfx/geometry/point.h"

#include "shared/test/mock_screen_observer.h"

#include "display_form_factor.h"
#include "hybris_utils.h"
#include "screen.h"
#include "screen_observer.h"
#include "shell_mode.h"

namespace oxide {

#if defined(ENABLE_HYBRIS)

class FakeHybrisUtils : public HybrisUtils {
 public:
  FakeHybrisUtils(bool has_properties)
      : has_properties_(has_properties) {
    OverrideForTesting(this);
  }

  ~FakeHybrisUtils() override { OverrideForTesting(nullptr); }

 private:
  // HybrisUtils implementation
  bool HasDeviceProperties() override { return has_properties_; }
  const DeviceProperties& GetDeviceProperties() override {
    static DeviceProperties g_props;
    return g_props;
  }
  bool IsUsingAndroidEGL() override { return false; }
#if defined(ENABLE_HYBRIS_CAMERA)
  bool IsCameraCompatAvailable() override { return false; }
#endif

  bool has_properties_;
};

#endif

class FakeScreen : public Screen {
 public:
  void TestPrimaryDisplayChanged() { NotifyPrimaryDisplayChanged(); }
  void TestDisplayAdded(const display::Display& display) {
    NotifyDisplayAdded(display);
  }
  void TestDisplayRemoved(const display::Display& display) {
    NotifyDisplayRemoved(display);
  }
  void TestDisplayPropertiesChanged(const display::Display& display) {
    NotifyDisplayPropertiesChanged(display);
  }
  void TestShellModeChanged() {
    NotifyShellModeChanged();
  }

 private:
  // Screen implementation
  display::Display GetPrimaryDisplay() override { return display::Display(1); }
  std::vector<display::Display> GetAllDisplays() override {
    return std::vector<display::Display>();
  }
  gfx::Point GetCursorScreenPoint() override { return gfx::Point(); }
};

class ScreenTest : public testing::Test {
 public:
  ScreenTest();

 protected:
  FakeScreen* screen() const { return screen_.get(); }

  void ResetScreen() { screen_.reset(); }

 private:
  std::unique_ptr<FakeScreen> screen_;
};

ScreenTest::ScreenTest()
    : screen_(new FakeScreen()) {}

TEST_F(ScreenTest, PrimaryDisplayChanged) {
  MockScreenObserver obs1;
  MockScreenObserver obs2;

  EXPECT_CALL(obs1, OnPrimaryDisplayChanged());
  EXPECT_CALL(obs2, OnPrimaryDisplayChanged());

  screen()->TestPrimaryDisplayChanged();
}

TEST_F(ScreenTest, DisplayAdded) {
  MockScreenObserver obs1;
  MockScreenObserver obs2;

  display::Display display(5);

  EXPECT_CALL(obs1, OnDisplayAdded(DisplayEq(display)));
  EXPECT_CALL(obs2, OnDisplayAdded(DisplayEq(display)));

  screen()->TestDisplayAdded(display);
}

TEST_F(ScreenTest, DisplayRemoved) {
  MockScreenObserver obs1;
  MockScreenObserver obs2;

  display::Display display(6);

  EXPECT_CALL(obs1, OnDisplayRemoved(DisplayEq(display)));
  EXPECT_CALL(obs2, OnDisplayRemoved(DisplayEq(display)));

  screen()->TestDisplayRemoved(display);
}

TEST_F(ScreenTest, DisplayPropertyChanged) {
  MockScreenObserver obs1;
  MockScreenObserver obs2;

  display::Display display(6);

  EXPECT_CALL(obs1, OnDisplayPropertiesChanged(DisplayEq(display)));
  EXPECT_CALL(obs2, OnDisplayPropertiesChanged(DisplayEq(display)));

  screen()->TestDisplayPropertiesChanged(display);
}

TEST_F(ScreenTest, ShellModeChanged) {
  MockScreenObserver obs1;
  MockScreenObserver obs2;

  EXPECT_CALL(obs1, OnShellModeChanged());
  EXPECT_CALL(obs2, OnShellModeChanged());

  screen()->TestShellModeChanged();
}

TEST_F(ScreenTest, Removal) {
  MockScreenObserver obs1;

  {
    MockScreenObserver obs2;

    EXPECT_CALL(obs1, OnPrimaryDisplayChanged());
    EXPECT_CALL(obs2, OnPrimaryDisplayChanged());

    screen()->TestPrimaryDisplayChanged();
  }

  EXPECT_CALL(obs1, OnPrimaryDisplayChanged());

  screen()->TestPrimaryDisplayChanged();
}

TEST_F(ScreenTest, ScreenDeleted) {
  MockScreenObserver obs1;
  MockScreenObserver obs2;
  MockScreenObserver obs3;
  MockScreenObserver obs4;
  MockScreenObserver obs5;
  MockScreenObserver obs6;
  MockScreenObserver obs7;
  MockScreenObserver obs8;
  MockScreenObserver obs9;
  MockScreenObserver obs10;

  ResetScreen();
  EXPECT_EQ(nullptr, Screen::GetInstance());
  // Shouldn't crash
}

TEST_F(ScreenTest, GetShellMode) {
#if defined(ENABLE_HYBRIS)
  {
    FakeHybrisUtils hybris_utils(false);
    EXPECT_EQ(ShellMode::Windowed, Screen::GetInstance()->GetShellMode());
  }

  {
    FakeHybrisUtils hybris_utils(true);
    EXPECT_EQ(ShellMode::NonWindowed, Screen::GetInstance()->GetShellMode());
  }
#else
  EXPECT_EQ(ShellMode::Windowed, Screen::GetInstance()->GetShellMode());
#endif
}

TEST_F(ScreenTest, GetDisplayFormFactor) {
#if defined(ENABLE_HYBRIS)
  {
    FakeHybrisUtils hybris_utils(false);
    EXPECT_EQ(DisplayFormFactor::Monitor,
              Screen::GetInstance()->GetDisplayFormFactor(
                  Screen::GetInstance()->GetPrimaryDisplay()));
    EXPECT_EQ(DisplayFormFactor::Monitor,
              Screen::GetInstance()->GetDisplayFormFactor(display::Display(2)));
  }

  {
    FakeHybrisUtils hybris_utils(true);
    EXPECT_EQ(DisplayFormFactor::Mobile,
              Screen::GetInstance()->GetDisplayFormFactor(
                  Screen::GetInstance()->GetPrimaryDisplay()));
    EXPECT_EQ(DisplayFormFactor::Monitor,
              Screen::GetInstance()->GetDisplayFormFactor(display::Display(2)));
  }
#else
  EXPECT_EQ(DisplayFormFactor::Monitor,
            Screen::GetInstance()->GetDisplayFormFactor(
                Screen::GetInstance()->GetPrimaryDisplay()));
  EXPECT_EQ(DisplayFormFactor::Monitor,
            Screen::GetInstance()->GetDisplayFormFactor(display::Display(2)));
#endif
}

} // namespace oxide
