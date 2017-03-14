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

#ifndef _OXIDE_SHARED_TEST_TEST_WEB_CONTENTS_FACTORY_H_
#define _OXIDE_SHARED_TEST_TEST_WEB_CONTENTS_FACTORY_H_

#include <memory>

#include "base/macros.h"

namespace content {
class BrowserContext;
class TestWebContentsFactory;
class WebContents;
}

namespace oxide {

// A wrapper around content::TestWebContentsFactory that does some additional
// Oxide-specific bootstrapping
class TestWebContentsFactory {
 public:
  TestWebContentsFactory();
  ~TestWebContentsFactory();
  
  content::WebContents* CreateWebContents(content::BrowserContext* context);

 private:
  std::unique_ptr<content::TestWebContentsFactory> factory_;

  DISALLOW_COPY_AND_ASSIGN(TestWebContentsFactory);
};

} // namespace oxide

#endif // _OXIDE_SHARED_TEST_TEST_WEB_CONTENTS_FACTORY_H_
