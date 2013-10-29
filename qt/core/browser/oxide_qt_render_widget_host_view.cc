// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#include "oxide_qt_render_widget_host_view.h"

#include <QGuiApplication>
#include <QRect>
#include <QScreen>

#include "third_party/WebKit/public/web/WebScreenInfo.h"
#include "ui/gfx/rect.h"

#include "qt/core/glue/oxide_qt_render_widget_host_view_delegate.h"

#include "oxide_qt_backing_store.h"

QT_USE_NAMESPACE

namespace oxide {
namespace qt {

void RenderWidgetHostView::ScheduleUpdate(const gfx::Rect& rect) {
  delegate_->ScheduleUpdate(
      QRect(rect.x(), rect.y(), rect.width(), rect.height()));
}

RenderWidgetHostView::RenderWidgetHostView(
    content::RenderWidgetHost* render_widget_host,
    RenderWidgetHostViewDelegate* delegate) :
    oxide::RenderWidgetHostView(render_widget_host),
    backing_store_(NULL),
    delegate_(delegate) {
  delegate_->SetRenderWidgetHostView(this);
}

RenderWidgetHostView::~RenderWidgetHostView() {}

// static
void RenderWidgetHostView::GetScreenInfo(
    QScreen* screen, WebKit::WebScreenInfo* result) {
  if (!screen) {
    screen = QGuiApplication::primaryScreen();
  }

  result->depth = screen->depth();
  result->depthPerComponent = 8; // XXX: Copied the GTK impl here
  result->isMonochrome = result->depth == 1;

  QRect rect = screen->geometry();
  result->rect = WebKit::WebRect(rect.x(),
                                 rect.y(),
                                 rect.width(),
                                 rect.height());

  QRect availableRect = screen->availableGeometry();
  result->availableRect = WebKit::WebRect(availableRect.x(),
                                          availableRect.y(),
                                          availableRect.width(),
                                          availableRect.height());
}

void RenderWidgetHostView::Blur() {
  delegate_->Blur();
}

void RenderWidgetHostView::Focus() {
  delegate_->Focus();
}

bool RenderWidgetHostView::HasFocus() const {
  return delegate_->HasFocus();
}

void RenderWidgetHostView::Show() {
  delegate_->Show();
}

void RenderWidgetHostView::Hide() {
  delegate_->Hide();
}

bool RenderWidgetHostView::IsShowing() {
  return delegate_->IsShowing();
}

gfx::Rect RenderWidgetHostView::GetViewBounds() const {
  QRect rect(delegate_->GetViewBounds());
  return gfx::Rect(rect.x(), rect.y(), rect.width(), rect.height());
}

content::BackingStore* RenderWidgetHostView::AllocBackingStore(
    const gfx::Size& size) {
  return new BackingStore(GetRenderWidgetHost(), size);
}

void RenderWidgetHostView::GetScreenInfo(
    WebKit::WebScreenInfo* results) {
  GetScreenInfo(delegate_->GetScreen(), results);
}

gfx::Rect RenderWidgetHostView::GetBoundsInRootWindow() {
  QRect rect(delegate_->GetBoundsInRootWindow());
  return gfx::Rect(rect.x(), rect.y(), rect.width(), rect.height());
}

} // namespace qt
} // namespace oxide
