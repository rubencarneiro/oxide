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

#include "legacy_touch_editing_client_proxy.h"

#include "ui/touch_selection/touch_selection_controller.h"

#include "qt/core/glue/legacy_touch_editing_client.h"
#include "qt/core/glue/macros.h"
#include "shared/browser/legacy_touch_editing_controller.h"

#include "contents_view_impl.h"
#include "oxide_qt_dpi_utils.h"
#include "oxide_qt_type_conversions.h"

namespace oxide {
namespace qt {

void LegacyTouchEditingClientProxy::StatusChanged(
    ui::TouchSelectionController::ActiveStatus status,
    const gfx::RectF& bounds,
    bool handle_drag_in_progress) {
  STATIC_ASSERT_MATCHING_ENUM(
      qt::LegacyTouchEditingClient::ActiveStatus::INACTIVE,
      ui::TouchSelectionController::INACTIVE);
  STATIC_ASSERT_MATCHING_ENUM(
      qt::LegacyTouchEditingClient::ActiveStatus::INSERTION_ACTIVE,
      ui::TouchSelectionController::INSERTION_ACTIVE);
  STATIC_ASSERT_MATCHING_ENUM(
      qt::LegacyTouchEditingClient::ActiveStatus::SELECTION_ACTIVE,
      ui::TouchSelectionController::SELECTION_ACTIVE);

  client_->StatusChanged(
      static_cast<qt::LegacyTouchEditingClient::ActiveStatus>(status),
      ToQt(DpiUtils::ConvertChromiumPixelsToQt(bounds, view_->GetScreen())),
      handle_drag_in_progress);
}

void LegacyTouchEditingClientProxy::InsertionHandleTapped() {
  client_->InsertionHandleTapped();
}

void LegacyTouchEditingClientProxy::ContextMenuIntercepted() {
  client_->ContextMenuIntercepted();
}

void LegacyTouchEditingClientProxy::HideAndDisallowShowingAutomatically() {
  controller()->HideAndDisallowShowingAutomatically();
}

LegacyTouchEditingClientProxy::LegacyTouchEditingClientProxy(
    ContentsViewImpl* view,
    qt::LegacyTouchEditingClient* client)
    : view_(view),
      client_(client) {
  qt::LegacyTouchEditingController::AttachToClient(client);
}

LegacyTouchEditingClientProxy::~LegacyTouchEditingClientProxy() = default;

} // namespace qt
} // namespace oxide
