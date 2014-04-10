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

#ifndef _OXIDE_QT_CORE_BROWSER_RENDER_WIDGET_HOST_VIEW_FACTORY_H_
#define _OXIDE_QT_CORE_BROWSER_RENDER_WIDGET_HOST_VIEW_FACTORY_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"

#include "shared/browser/oxide_render_widget_host_view_factory.h"

namespace oxide {
namespace qt {

class RenderWidgetHostViewDelegateFactory;

class RenderWidgetHostViewFactory FINAL :
    public oxide::RenderWidgetHostViewFactory {
 public:
  RenderWidgetHostViewFactory(oxide::BrowserContext* context,
                              RenderWidgetHostViewDelegateFactory* delegate);
  ~RenderWidgetHostViewFactory();

  oxide::RenderWidgetHostView* CreateViewForWidget(
      content::RenderWidgetHost* render_widget_host) FINAL;

 private:
  scoped_ptr<RenderWidgetHostViewDelegateFactory> delegate_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(RenderWidgetHostViewFactory);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_RENDER_WIDGET_HOST_VIEW_FACTORY_H_
