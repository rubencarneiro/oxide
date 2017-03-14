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

#include "test_web_contents_factory.h"

#include "base/memory/ptr_util.h"
#include "content/public/test/test_web_contents_factory.h"

#include "oxide_test_web_contents_view.h"

namespace oxide {

TestWebContentsFactory::TestWebContentsFactory() {
  content::SetWebContentsViewOxideFactory(TestWebContentsView::Create);
  factory_ = base::MakeUnique<content::TestWebContentsFactory>();
}

TestWebContentsFactory::~TestWebContentsFactory() {
  factory_.reset();
  content::SetWebContentsViewOxideFactory(nullptr);
}

content::WebContents* TestWebContentsFactory::CreateWebContents(
    content::BrowserContext* context) {
  return factory_->CreateWebContents(context);
}

} // namespace oxide
