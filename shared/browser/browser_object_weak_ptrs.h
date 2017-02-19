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

#ifndef _OXIDE_SHARED_BROWSER_BROWSER_OBJECT_WEAK_PTRS_H_
#define _OXIDE_SHARED_BROWSER_BROWSER_OBJECT_WEAK_PTRS_H_

#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_widget_host.h"

#include "shared/common/render_object_weak_ptr.h"

namespace oxide {

namespace internal {

template <class T>
struct BrowserObjectWeakPtrTraits {
  struct ID {
    ID() = default;
    ID(int first, int second)
        : first(first), second(second) {}

    int first = MSG_ROUTING_NONE;
    int second = MSG_ROUTING_NONE;
  };

  static ID GetID(T* p) {
    return ID(p->GetProcess()->GetID(), p->GetRoutingID());
  }

  static bool IsValidID(const ID& id) {
    return !!T::FromID(id.first, id.second);
  }
};

template <>
struct RenderObjectWeakPtrTraits<content::RenderFrameHost>
    : BrowserObjectWeakPtrTraits<content::RenderFrameHost> {};

template <>
struct RenderObjectWeakPtrTraits<content::RenderWidgetHost>
    : BrowserObjectWeakPtrTraits<content::RenderWidgetHost> {};

} // namespace internal

using RenderFrameHostWeakPtr = RenderObjectWeakPtr<content::RenderFrameHost>;
using RenderWidgetHostWeakPtr = RenderObjectWeakPtr<content::RenderWidgetHost>;

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_BROWSER_OBJECT_WEAK_PTRS_H_
