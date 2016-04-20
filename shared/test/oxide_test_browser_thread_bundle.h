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

#ifndef _OXIDE_SHARED_TEST_TEST_BROWSER_THREAD_BUNDLE_H_
#define _OXIDE_SHARED_TEST_TEST_BROWSER_THREAD_BUNDLE_H_

#include <memory>

#include "base/macros.h"

namespace content {
class TestBrowserThreadBundle;
}

namespace oxide {

// A helper class around content::TestBrowserThreadBundle that creates the
// UI thread MessageLoop with a MessagePumpDefault for unit tests, rather than
// using the Chromium MessagePumpForUI implementation, which we don't normally
// use in Oxide (we use a MessagePump provided by the embedding layer)
class TestBrowserThreadBundle {
 public:
  TestBrowserThreadBundle();
  ~TestBrowserThreadBundle();

 private:
  std::unique_ptr<content::TestBrowserThreadBundle> bundle_;

  DISALLOW_COPY_AND_ASSIGN(TestBrowserThreadBundle);
};

} // namespace oxide

#endif // _OXIDE_SHARED_TEST_TEST_BROWSER_THREAD_BUNDLE_H_
