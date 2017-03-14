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

#ifndef _OXIDE_SHARED_TEST_MOCK_SCREEN_OBSERVER_H_
#define _OXIDE_SHARED_TEST_MOCK_SCREEN_OBSERVER_H_

#include "base/macros.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "ui/display/display.h"
#include "ui/gfx/geometry/rect.h"

#include "shared/browser/screen_observer.h"

namespace oxide {

class MockScreenObserver : public ScreenObserver {
 public:
  // ScreenObserver implementation
  MOCK_METHOD0(OnPrimaryDisplayChanged, void());
  MOCK_METHOD1(OnDisplayAdded, void(const display::Display& display));
  MOCK_METHOD1(OnDisplayRemoved, void(const display::Display& display));
  MOCK_METHOD1(OnDisplayPropertiesChanged, void(const display::Display& display));
  MOCK_METHOD0(OnShellModeChanged, void());
};

class DisplayEqMatcher
    : public testing::MatcherInterface<const display::Display&> {
 public:
  DisplayEqMatcher(const display::Display& expected)
      : expected_(expected) {}

  bool MatchAndExplain(const display::Display& actual,
                       testing::MatchResultListener* listener) const override {
    *listener << "the Display is " << actual.ToString() << " (expected " <<
                 expected_.ToString() << ")";
    return actual.id() == expected_.id() &&
           actual.bounds() == expected_.bounds() &&
           actual.work_area() == expected_.work_area() &&
           actual.device_scale_factor() == expected_.device_scale_factor() &&
           actual.rotation() == expected_.rotation() &&
           actual.touch_support() == expected_.touch_support();
  }

  void DescribeTo(std::ostream* os) const override {
    *os << "which matches the expected display";
  }

  void DescribeNegationTo(std::ostream* os) const override {
    *os << "which does not match the expected display";
  }

 private:
  display::Display expected_;

  DISALLOW_COPY_AND_ASSIGN(DisplayEqMatcher);
};

testing::Matcher<const display::Display&> DisplayEq(
    const display::Display& expected) {
  return testing::MakeMatcher(new DisplayEqMatcher(expected));
}

} // namespace oxide

#endif // _OXIDE_SHARED_TEST_MOCK_SCREEN_OBSERVER_H_
