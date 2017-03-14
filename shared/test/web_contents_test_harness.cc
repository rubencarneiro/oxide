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

#include "web_contents_test_harness.h"

#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted.h"
#include "base/run_loop.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/site_instance.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/test_browser_context.h"
#include "content/public/test/test_renderer_host.h"
#include "content/public/test/web_contents_tester.h"

#include "oxide_test_web_contents_view.h"
#include "test_browser_thread_bundle.h"

namespace oxide {

void WebContentsTestHarness::SetUp() {
  browser_thread_bundle_ = base::MakeUnique<TestBrowserThreadBundle>();

  content::SetWebContentsViewOxideFactory(TestWebContentsView::Create);
  rvh_test_enabler_ = base::MakeUnique<content::RenderViewHostTestEnabler>();

  browser_context_ = base::MakeUnique<content::TestBrowserContext>();

  scoped_refptr<content::SiteInstance> instance =
      content::SiteInstance::Create(browser_context_.get());
  instance->GetProcess()->Init();

  web_contents_ = CreateTestWebContents();
}

void WebContentsTestHarness::TearDown() {
  web_contents_.reset();

  base::RunLoop().RunUntilIdle();

  rvh_test_enabler_.reset();

  content::BrowserThread::DeleteSoon(content::BrowserThread::UI,
                                     FROM_HERE,
                                     browser_context_.release());
  browser_thread_bundle_.reset();

  content::SetWebContentsViewOxideFactory(nullptr);
}

WebContentsTestHarness::WebContentsTestHarness() = default;

WebContentsTestHarness::~WebContentsTestHarness() = default;

std::unique_ptr<content::WebContents>
WebContentsTestHarness::CreateTestWebContents() {
  scoped_refptr<content::SiteInstance> instance =
      content::SiteInstance::Create(browser_context_.get());
  instance->GetProcess()->Init();

  return base::WrapUnique(
      content::WebContentsTester::CreateTestWebContents(browser_context_.get(),
                                                        std::move(instance)));
}

} // namespace oxide
