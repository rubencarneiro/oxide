// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include "oxide_qt_render_widget_host_view_factory.h"

#include "qt/core/glue/oxide_qt_render_widget_host_view_delegate_factory.h"

#include "oxide_qt_render_widget_host_view.h"

namespace oxide {
namespace qt {

RenderWidgetHostViewFactory::RenderWidgetHostViewFactory(
    oxide::BrowserContext* context,
    RenderWidgetHostViewDelegateFactory* delegate) :
    oxide::RenderWidgetHostViewFactory(context),
    delegate_(delegate) {}

RenderWidgetHostViewFactory::~RenderWidgetHostViewFactory() {}

oxide::RenderWidgetHostView* RenderWidgetHostViewFactory::CreateViewForWidget(
    content::RenderWidgetHost* render_widget_host) {
  return new RenderWidgetHostView(
      render_widget_host,
      delegate_->CreateRenderWidgetHostViewDelegate());
}

} // namespace qt
} // namespace oxide
