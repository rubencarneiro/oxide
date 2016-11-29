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

#include "uitk_web_context_menu.h"

#include <QDebug>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickItem>
#include <QRect>
#include <QUrl>
#include <QVariant>

#include "qt/core/glue/web_context_menu_actions.h"
#include "qt/core/glue/web_context_menu_client.h"
#include "qt/core/glue/web_context_menu_params.h"

namespace oxide {
namespace uitk {

using qt::MenuItem;
using qt::WebContextMenuAction;
using qt::WebContextMenuClient;
using qt::WebContextMenuParams;

WebContextMenu::WebContextMenu(QQuickItem* parent,
                               WebContextMenuClient* client)
    : parent_(parent),
      client_(client) {}

bool WebContextMenu::Init(const std::vector<MenuItem>& items,
                          const WebContextMenuParams& params,
                          bool mobile) {
  QQmlEngine* engine = qmlEngine(parent_);
  if (!engine) {
    qWarning() <<
        "uitk::WebContextMenu: Failed to initialize - cannot determine "
        "QQmlEngine for parent item";
    return false;
  }

  QQmlComponent action_component(engine);
  action_component.setData("import Ubuntu.Components 1.3; Action {}", QUrl());
  if (action_component.isError()) {
    qCritical() <<
        "Failed to initialize Action component because of the following "
        "errors: ";
    for (const auto& error : action_component.errors()) {
      qCritical() << error;
    }
    return false;
  }

  Q_ASSERT(action_component.isReady());

  for (const auto& item : items) {
    if (item.separator) {
      continue;
    }

    QObject* action = action_component.beginCreate(engine->rootContext());
    if (!action) {
      qCritical() << "Failed to create Action instance";
      return false;
    }

    action->setProperty("text", item.label);
    action->setProperty("enabled", item.enabled);
    action_component.completeCreate();

    action->setProperty("__action", item.action);

    connect(action, SIGNAL(triggered(const QVariant&)),
            this, SLOT(OnActionTriggered(const QVariant&)));

    actions_.push_back(std::unique_ptr<QObject>(action));
  }

  if (actions_.empty()) {
    return false;
  }

  QQmlComponent menu_component(engine);
  if (mobile) {
    menu_component.loadUrl(QUrl("qrc:///WebContextMenuMobile.qml"));
  } else {
    menu_component.loadUrl(QUrl("qrc:///WebContextMenuDesktop.qml"));
  }
  if (menu_component.isError()) {
    qCritical() <<
        "Failed to initialize context menu component because of the following "
        "errors: ";
    for (const auto& error : menu_component.errors()) {
      qCritical() << error;
    }
    return false;
  }

  Q_ASSERT(menu_component.isReady());

  QObject* menu = menu_component.beginCreate(engine->rootContext());
  if (!menu) {
    qCritical() << "Failed to create context menu instance";
    return false;
  }

  item_.reset(qobject_cast<QQuickItem*>(menu));
  if (!item_) {
    qCritical() << "Context menu instance is not a QQuickItem";
    delete menu;
    return false;
  }

  QList<QObject*> actions;
  for (const auto& action : actions_) {
    actions.push_back(action.get());
  }

  item_->setProperty("actions", QVariant::fromValue(actions));
  item_->setProperty("isImage",
                     params.media_type == qt::MEDIA_TYPE_IMAGE ||
                         params.media_type == qt::MEDIA_TYPE_CANVAS);
  item_->setProperty("openerName", parent_->objectName());
  item_->setProperty("position", params.position);
  item_->setProperty("sourceItem", QVariant::fromValue(parent_.data()));
  item_->setProperty("title",
                     !params.src_url.isEmpty() ?
                         params.src_url : params.link_url);

  // PopupBase.qml reparents the menu to the root item, but we need to set a
  // parent item here first so that it can find the root
  item_->setParentItem(parent_);

  menu_component.completeCreate();

  connect(item_.get(), &QQuickItem::visibleChanged,
          this, &WebContextMenu::OnVisibleChanged);
  connect(item_.get(), SIGNAL(cancelled()),
          this, SLOT(OnCancelled()));

  return true;
}

void WebContextMenu::Show() {
  QMetaObject::invokeMethod(item_.get(), "show");
}

void WebContextMenu::Hide() {
  QMetaObject::invokeMethod(item_.get(), "hide");
  // See https://launchpad.net/bugs/1526884
  parent_->forceActiveFocus();
}

void WebContextMenu::OnVisibleChanged() {
  if (!item_->isVisible()) {
    client_->close();
  }
}

void WebContextMenu::OnCancelled() {
  client_->close();
}

void WebContextMenu::OnActionTriggered(const QVariant&) {
  unsigned action = sender()->property("__action").toUInt();
  client_->execCommand(static_cast<WebContextMenuAction>(action));
  client_->close();
}

// static
std::unique_ptr<WebContextMenu> WebContextMenu::Create(
    QQuickItem* parent,
    const WebContextMenuParams& params,
    const std::vector<MenuItem>& items,
    WebContextMenuClient* client,
    bool mobile) {
  std::unique_ptr<WebContextMenu> menu(new WebContextMenu(parent, client));
  if (!menu->Init(items, params, mobile)) {
    return nullptr;
  }

  return std::move(menu);
}

WebContextMenu::~WebContextMenu() = default;

} // namespace uitk
} // namespace oxide
