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

#include "qquick_legacy_auxiliary_ui_factory.h"

#include <memory>

#include "qt/core/glue/javascript_dialog_type.h"
#include "qt/core/glue/touch_editing_menu.h"

#include "qquick_legacy_alert_dialog.h"
#include "qquick_legacy_before_unload_dialog.h"
#include "qquick_legacy_confirm_dialog.h"
#include "qquick_legacy_prompt_dialog.h"
#include "qquick_legacy_web_context_menu.h"

namespace oxide {
namespace qquick {

using qt::EditCapabilityFlags;
using qt::JavaScriptDialogClient;
using qt::JavaScriptDialogType;
using qt::MenuItem;
using qt::TouchEditingMenuClient;
using qt::WebContextMenuClient;
using qt::WebContextMenuParams;

std::unique_ptr<qt::WebContextMenu>
LegacyAuxiliaryUIFactory::CreateWebContextMenu(
    const WebContextMenuParams& params,
    const std::vector<MenuItem>& items,
    WebContextMenuClient* client) {
  if (!context_menu_) {
    return nullptr;
  }

  return std::unique_ptr<qt::WebContextMenu>(
      new LegacyWebContextMenu(item_, context_menu_, params, client));
}

std::unique_ptr<qt::TouchEditingMenu>
LegacyAuxiliaryUIFactory::CreateTouchEditingMenu(
    EditCapabilityFlags edit_flags,
    TouchEditingMenuClient* client) {
  Q_ASSERT(0);
  return nullptr;
}

std::unique_ptr<qt::JavaScriptDialog>
LegacyAuxiliaryUIFactory::CreateJavaScriptDialog(
    const QUrl& origin_url,
    JavaScriptDialogType type,
    const QString& message_text,
    const QString& default_prompt_text,
    JavaScriptDialogClient* client) {
  switch (type) {
    case JavaScriptDialogType::Alert:
      if (!alert_dialog_) {
        return nullptr;
      }
      return std::unique_ptr<qt::JavaScriptDialog>(
          new LegacyAlertDialog(item_, alert_dialog_, message_text, client));
    case JavaScriptDialogType::Confirm:
      if (!confirm_dialog_) {
        return nullptr;
      }
      return std::unique_ptr<qt::JavaScriptDialog>(
          new LegacyConfirmDialog(item_, confirm_dialog_,
                                  message_text, client));
    case JavaScriptDialogType::Prompt:
      if (!prompt_dialog_) {
        return nullptr;
      }
      return std::unique_ptr<qt::JavaScriptDialog>(
          new LegacyPromptDialog(item_, prompt_dialog_, message_text,
                                 default_prompt_text, client));
  }
}

std::unique_ptr<qt::JavaScriptDialog>
LegacyAuxiliaryUIFactory::CreateBeforeUnloadDialog(
    const QUrl& origin_url,
    JavaScriptDialogClient* client) {
  if (!before_unload_dialog_) {
    return nullptr;
  }

  return std::unique_ptr<qt::JavaScriptDialog>(
      new LegacyBeforeUnloadDialog(item_, before_unload_dialog_, client));
}

LegacyAuxiliaryUIFactory::LegacyAuxiliaryUIFactory(QQuickItem* item)
    : item_(item) {}

LegacyAuxiliaryUIFactory::~LegacyAuxiliaryUIFactory() = default;

} // namespace qquick
} // namespace oxide
