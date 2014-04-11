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

#ifndef _OXIDE_QQUICK_RENDER_VIEW_ITEM_FACTORY_H_
#define _OXIDE_QQUICK_RENDER_VIEW_ITEM_FACTORY_H_

#include <QtGlobal>

#include "qt/core/glue/oxide_qt_render_widget_host_view_delegate_factory.h"

namespace oxide {
namespace qquick {

class RenderViewItemFactory Q_DECL_FINAL :
    public oxide::qt::RenderWidgetHostViewDelegateFactory {
 public:
  RenderViewItemFactory();

  oxide::qt::RenderWidgetHostViewDelegate*
  CreateRenderWidgetHostViewDelegate() Q_DECL_FINAL;
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QQUICK_RENDER_VIEW_ITEM_FACTORY_H_
