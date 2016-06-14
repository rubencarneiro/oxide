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

#include "oxide_test_suite.h"

#include "content/browser/web_contents/web_contents_view_oxide.h" // nogncheck
#include "content/public/test/test_content_client_initializer.h"
#include "testing/gtest/include/gtest/gtest.h"

#include "oxide_test_web_contents_view.h"

namespace oxide {

class TestInitializationListener : public testing::EmptyTestEventListener {
 public:
  TestInitializationListener()
      : test_content_client_initializer_(nullptr) {}

  void OnTestStart(const testing::TestInfo& test_info) override {
    test_content_client_initializer_ =
        new content::TestContentClientInitializer();
  }

  void OnTestEnd(const testing::TestInfo& test_info) override {
    delete test_content_client_initializer_;
  }

 private:
  content::TestContentClientInitializer* test_content_client_initializer_;

  DISALLOW_COPY_AND_ASSIGN(TestInitializationListener);
};

void TestSuite::Initialize() {
  content::SetWebContentsViewOxideFactory(oxide::TestWebContentsView::Create);

  content::ContentTestSuiteBase::Initialize();

  testing::TestEventListeners& listeners =
      testing::UnitTest::GetInstance()->listeners();
  listeners.Append(new TestInitializationListener());
}

TestSuite::TestSuite(int argc, char** argv)
    : content::ContentTestSuiteBase(argc, argv) {}

TestSuite::~TestSuite() {}

} // namespace oxide
