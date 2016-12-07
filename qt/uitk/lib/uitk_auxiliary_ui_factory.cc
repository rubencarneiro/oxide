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

#include "uitk_auxiliary_ui_factory.h"

#include <memory>

#include <QDebug>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickItem>
#include <QQuickWindow>
#include <QScreen>

#include "qt/core/glue/screen_utils.h"
#include "qt/core/glue/web_context_menu_actions.h"
#include "qt/core/glue/web_context_menu_sections.h"
#include "qt/uitk/lib/api/oxideubuntuwebcontextmenu.h"
#include "qt/uitk/lib/api/oxideubuntuwebcontextmenu_p.h"
#include "qt/uitk/lib/api/oxideubuntuwebcontextmenuitem.h"
#include "qt/uitk/lib/api/oxideubuntuwebcontextmenuitem_p.h"

#include "uitk_web_context_menu.h"

static void InitResources() {
  Q_INIT_RESOURCE(resources);
}

namespace oxide {
namespace uitk {

using qt::GetScreenFormFactor;
using qt::MenuItem;
using qt::ScreenFormFactor;
using qt::WebContextMenuAction;
using qt::WebContextMenuClient;
using qt::WebContextMenuParams;
using qt::WebContextMenuSection;

namespace {

bool IsItemOnMobileScreen(QQuickItem* item) {
  QQuickWindow* window = item->window();
  if (!window) {
    return false;
  }

  QScreen* screen = window->screen();
  if (!screen) {
    return false;
  }

  return GetScreenFormFactor(screen) == ScreenFormFactor::Mobile;
}

OxideUbuntuWebContextMenuItem::Section ToSection(unsigned i) {
  Q_ASSERT(i >= static_cast<unsigned>(WebContextMenuSection::Min));
  Q_ASSERT(i < static_cast<unsigned>(WebContextMenuSection::Max));

  WebContextMenuSection section = static_cast<WebContextMenuSection>(i);
  switch (section) {
    case WebContextMenuSection::OpenLink:
      return OxideUbuntuWebContextMenuItem::SectionOpenLink;
    case WebContextMenuSection::Link:
      return OxideUbuntuWebContextMenuItem::SectionLink;
    case WebContextMenuSection::Media:
      return OxideUbuntuWebContextMenuItem::SectionMedia;
    case WebContextMenuSection::Undo:
      return OxideUbuntuWebContextMenuItem::SectionUndo;
    case WebContextMenuSection::Editing:
      return OxideUbuntuWebContextMenuItem::SectionEditing;
    case WebContextMenuSection::Copy:
      return OxideUbuntuWebContextMenuItem::SectionCopy;
  }

  Q_UNREACHABLE();
}

OxideUbuntuWebContextMenuItem::Action ToAction(unsigned i) {
  Q_ASSERT(i >= static_cast<unsigned>(WebContextMenuAction::Min));
  Q_ASSERT(i < static_cast<unsigned>(WebContextMenuAction::Max));

  WebContextMenuAction action = static_cast<WebContextMenuAction>(i);
  switch (action) {
    case WebContextMenuAction::OpenLinkInNewTab:
      return OxideUbuntuWebContextMenuItem::ActionOpenLinkInNewTab;
    case WebContextMenuAction::OpenLinkInNewBackgroundTab:
      return OxideUbuntuWebContextMenuItem::ActionOpenLinkInNewBackgroundTab;
    case WebContextMenuAction::OpenLinkInNewWindow:
      return OxideUbuntuWebContextMenuItem::ActionOpenLinkInNewWindow;
    case WebContextMenuAction::CopyLinkLocation:

      return OxideUbuntuWebContextMenuItem::ActionCopyLinkLocation;
    case WebContextMenuAction::SaveLink:
      return OxideUbuntuWebContextMenuItem::ActionSaveLink;

    case WebContextMenuAction::OpenImageInNewTab:
    case WebContextMenuAction::OpenMediaInNewTab:
      return OxideUbuntuWebContextMenuItem::ActionOpenMediaInNewTab;
    case WebContextMenuAction::CopyImageLocation:
    case WebContextMenuAction::CopyMediaLocation:
      return OxideUbuntuWebContextMenuItem::ActionCopyMediaLocation;
    case WebContextMenuAction::SaveImage:
    case WebContextMenuAction::SaveMedia:
      return OxideUbuntuWebContextMenuItem::ActionSaveMedia;
    case WebContextMenuAction::CopyImage:
      return OxideUbuntuWebContextMenuItem::ActionCopyImage;

    case WebContextMenuAction::Undo:
      return OxideUbuntuWebContextMenuItem::ActionUndo;
    case WebContextMenuAction::Redo:
      return OxideUbuntuWebContextMenuItem::ActionRedo;
    case WebContextMenuAction::Cut:
      return OxideUbuntuWebContextMenuItem::ActionCut;
    case WebContextMenuAction::Copy:
      return OxideUbuntuWebContextMenuItem::ActionCopy;
    case WebContextMenuAction::Paste:
      return OxideUbuntuWebContextMenuItem::ActionPaste;
    case WebContextMenuAction::Erase:
      return OxideUbuntuWebContextMenuItem::ActionErase;
    case WebContextMenuAction::SelectAll:
      return OxideUbuntuWebContextMenuItem::ActionSelectAll;
  }

  Q_UNREACHABLE();
}

}

AuxiliaryUIFactory::Delegate::~Delegate() = default;

std::unique_ptr<qt::WebContextMenu> AuxiliaryUIFactory::CreateWebContextMenu(
    const WebContextMenuParams& params,
    const std::vector<MenuItem>& items,
    WebContextMenuClient* client) {
  QQmlEngine* engine = qmlEngine(item());
  if (!engine) {
    qWarning() <<
        "uitk::AuxiliaryUIFactory: Failed to create context menu - cannot "
        "determine QQmlEngine for parent item";
    return nullptr;
  }

  QQmlComponent action_component(engine);
  action_component.setData("import Ubuntu.Components 1.3; Action {}", QUrl());
  if (action_component.isError()) {
    qCritical() <<
        "uitk::AuxiliaryUIFactory: Failed to initialize Action component "
        "because of the following errors: ";
    for (const auto& error : action_component.errors()) {
      qCritical() << error;
    }
    return nullptr;
  }

  Q_ASSERT(action_component.isReady());

  std::unique_ptr<OxideUbuntuWebContextMenu> menu(
      new OxideUbuntuWebContextMenu());
  OxideUbuntuWebContextMenuPrivate* menu_d =
      OxideUbuntuWebContextMenuPrivate::get(menu.get());

  std::vector<QObject*> stock_actions;

  OxideUbuntuWebContextMenuItem::Section current_section =
      OxideUbuntuWebContextMenuItem::NoSection;

  for (const auto& item : items) {
    if (item.type == MenuItem::Type::Group) {
      current_section = ToSection(item.action);
      continue;
    }

    Q_ASSERT(current_section != OxideUbuntuWebContextMenuItem::NoSection);
    Q_ASSERT(item.type == MenuItem::Type::Option);

    QObject* action = action_component.beginCreate(engine->rootContext());
    if (!action) {
      qCritical() <<
          "uitk::AuxiliaryUIFactory: Failed to create Action instance";
      return nullptr;
    }

    action->setProperty("text", item.label);
    action->setProperty("enabled", item.enabled);
    action_component.completeCreate();

    action->setProperty("__stock_action", item.action);
    stock_actions.push_back(action);

    // Takes ownership of the action
    OxideUbuntuWebContextMenuItem* i =
        OxideUbuntuWebContextMenuItemPrivate::CreateStockItem(
            ToAction(item.action),
            current_section,
            action);
    // Takes ownership of the item
    menu_d->AppendItem(i);
  }

  if (delegate_) {
    delegate_->ContextMenuOpening(params, menu.get());
  }

  if (menu->isEmpty()) {
    return nullptr;
  }

  return WebContextMenu::Create(engine, item(), params, std::move(menu),
                                stock_actions, client,
                                IsItemOnMobileScreen(item()));
}

AuxiliaryUIFactory::AuxiliaryUIFactory(QQuickItem* item, Delegate* delegate)
    : qquick::AuxiliaryUIFactory(item),
      delegate_(delegate) {
  InitResources();
}

AuxiliaryUIFactory::~AuxiliaryUIFactory() = default;

} // namespace uitk
} // namespace oxide
