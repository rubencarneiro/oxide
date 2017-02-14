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

#ifndef _OXIDE_QT_CORE_BROWSER_LEGACY_TOUCH_EDITING_CLIENT_PROXY_H_
#define _OXIDE_QT_CORE_BROWSER_LEGACY_TOUCH_EDITING_CLIENT_PROXY_H_

#include "base/macros.h"

#include "qt/core/glue/legacy_touch_editing_controller.h"
#include "shared/browser/legacy_touch_editing_client.h"

namespace oxide {
namespace qt {

class ContentsViewImpl;
class LegacyTouchEditingClient;

class LegacyTouchEditingClientProxy : public oxide::LegacyTouchEditingClient,
                                      public LegacyTouchEditingController {
 public:
  LegacyTouchEditingClientProxy(ContentsViewImpl* view,
                                qt::LegacyTouchEditingClient* client);
  ~LegacyTouchEditingClientProxy() override;

 private:
  // oxide::LegacyTouchEditingClient implementation
  void StatusChanged(ui::TouchSelectionController::ActiveStatus status,
                     const gfx::RectF& bounds,
                     bool handle_drag_in_progress) override;
  void InsertionHandleTapped() override;
  void ContextMenuIntercepted() override;

  // LegacyTouchEditingController implementation
  void HideAndDisallowShowingAutomatically() override;

  ContentsViewImpl* view_;
  qt::LegacyTouchEditingClient* client_;

  DISALLOW_COPY_AND_ASSIGN(LegacyTouchEditingClientProxy);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_LEGACY_TOUCH_EDITING_CLIENT_PROXY_H_
