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

#ifndef _OXIDE_SHARED_RENDERER_EXTERNAL_POPUP_MENU_H_
#define _OXIDE_SHARED_RENDERER_EXTERNAL_POPUP_MENU_H_

#include "base/macros.h"
#include "content/public/renderer/render_frame_observer.h"
#include "content/public/renderer/render_frame_observer_tracker.h"
#include "third_party/WebKit/public/web/WebExternalPopupMenu.h"
#include "third_party/WebKit/public/web/WebPopupMenuInfo.h"
#include "ui/gfx/geometry/point_f.h"

namespace blink {
class WebExternalPopupMenuClient;
}

namespace content {
class RenderFrame;
}

namespace oxide {

class ExternalPopupMenu
    : public blink::WebExternalPopupMenu,
      public content::RenderFrameObserver,
      public content::RenderFrameObserverTracker<ExternalPopupMenu> {
 public:
  ExternalPopupMenu(
      content::RenderFrame* render_frame,
      const blink::WebPopupMenuInfo& popup_menu_info,
      blink::WebExternalPopupMenuClient* popup_menu_client,
      float origin_scale_for_emulation,
      const gfx::PointF& origin_offset_for_emulation);
  ~ExternalPopupMenu() override;

 private:
  void OnDidSelectPopupMenuItems(const std::vector<int>& indices);
  void OnDidCancelPopupMenu();

  // blink::WebExternalPopupMenu implementation
  void show(const blink::WebRect& bounds) override;
  void close() override;

  // IPC::Listener implementation
  bool OnMessageReceived(const IPC::Message& message) override;

  blink::WebPopupMenuInfo popup_menu_info_;

  blink::WebExternalPopupMenuClient* client_;

  float origin_scale_for_emulation_;
  gfx::PointF origin_offset_for_emulation_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_EXTERNAL_POPUP_MENU_H_
