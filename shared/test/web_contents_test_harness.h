// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2017 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_TEST_WEB_CONTENTS_TEST_HARNESS_H_
#define _OXIDE_SHARED_TEST_WEB_CONTENTS_TEST_HARNESS_H_

#include <memory>

#include "base/macros.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace content {
class RenderViewHostTestEnabler;
class TestBrowserContext;
class WebContents;
}

namespace oxide {

class TestBrowserThreadBundle;

class WebContentsTestHarness : public testing::Test {
 public:
  WebContentsTestHarness();
  ~WebContentsTestHarness() override;

  content::WebContents* web_contents() const { return web_contents_.get(); }

  std::unique_ptr<content::WebContents> CreateTestWebContents();

 protected:
  void SetUp() override;
  void TearDown() override;

 private:
  std::unique_ptr<TestBrowserThreadBundle> browser_thread_bundle_;

  std::unique_ptr<content::RenderViewHostTestEnabler> rvh_test_enabler_;

  std::unique_ptr<content::TestBrowserContext> browser_context_;

  std::unique_ptr<content::WebContents> web_contents_;

  DISALLOW_COPY_AND_ASSIGN(WebContentsTestHarness);
};

} // namespace oxide

#endif // _OXIDE_SHARED_TEST_WEB_CONTENTS_TEST_HARNESS_H_
