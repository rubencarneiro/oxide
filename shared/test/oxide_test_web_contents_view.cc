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

#include "oxide_test_web_contents_view.h"

#include "base/logging.h"
#include "ui/gfx/geometry/rect.h"

namespace oxide {

TestWebContentsView::TestWebContentsView() {}

gfx::NativeView TestWebContentsView::GetNativeView() const {
  return nullptr;
}

gfx::NativeView TestWebContentsView::GetContentNativeView() const {
  return nullptr;
}

gfx::NativeWindow TestWebContentsView::GetTopLevelNativeWindow() const {
  return nullptr;
}

void TestWebContentsView::GetScreenInfo(
    blink::WebScreenInfo* web_screen_info) const {}

void TestWebContentsView::GetContainerBounds(gfx::Rect* out) const {
  *out = gfx::Rect(size_);
}

void TestWebContentsView::SizeContents(const gfx::Size& size) {
  size_ = size;
}

void TestWebContentsView::Focus() {}

void TestWebContentsView::SetInitialFocus() {}

void TestWebContentsView::StoreFocus() {}

void TestWebContentsView::RestoreFocus() {}

content::DropData* TestWebContentsView::GetDropData() const {
  return nullptr;
}

gfx::Rect TestWebContentsView::GetViewBounds() const {
  return gfx::Rect(size_);
}

void TestWebContentsView::CreateView(const gfx::Size& initial_size,
                                     gfx::NativeView context) {}

content::RenderWidgetHostViewBase* TestWebContentsView::CreateViewForWidget(
    content::RenderWidgetHost* render_widget_host,
    bool is_guest_view_hack) {
  NOTREACHED();
  return nullptr;
}

content::RenderWidgetHostViewBase*
TestWebContentsView::CreateViewForPopupWidget(
    content::RenderWidgetHost* render_widget_host) {
  NOTREACHED();
  return nullptr;
}

void TestWebContentsView::SetPageTitle(const base::string16& title) {}

void TestWebContentsView::RenderViewCreated(content::RenderViewHost* host) {}

void TestWebContentsView::RenderViewSwappedIn(content::RenderViewHost* host) {}

void TestWebContentsView::SetOverscrollControllerEnabled(bool enabled) {}

// static
content::WebContentsViewOxide* TestWebContentsView::Create(
    content::WebContents* contents) {
  return new TestWebContentsView();
}

TestWebContentsView::~TestWebContentsView() {}

} // namespace oxide
