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
#include "qt/uitk/lib/api/oxideubuntuwebcontextmenu.h"
#include "qt/uitk/lib/api/oxideubuntuwebcontextmenu_p.h"
#include "qt/uitk/lib/api/oxideubuntuwebcontextmenuitem.h"

namespace oxide {
namespace uitk {

using qt::WebContextMenuAction;
using qt::WebContextMenuClient;
using qt::WebContextMenuParams;

WebContextMenu::WebContextMenu(QQuickItem* parent,
                               std::unique_ptr<OxideUbuntuWebContextMenu> menu,
                               const std::vector<QObject*>& stock_actions,
                               WebContextMenuClient* client)
    : parent_(parent),
      client_(client),
      menu_(std::move(menu)) {
  for (const auto* stock_action : stock_actions) {
    connect(stock_action, SIGNAL(triggered(const QVariant&)),
            this, SLOT(OnStockActionTriggered(const QVariant&)));
  }
}

bool WebContextMenu::Init(QQmlEngine* engine,
                          const WebContextMenuParams& params,
                          bool mobile) {
  QQmlComponent menu_component(engine);
  if (mobile) {
    menu_component.loadUrl(QUrl("qrc:///WebContextMenuMobile.qml"));
  } else {
    menu_component.loadUrl(QUrl("qrc:///WebContextMenuDesktop.qml"));
  }
  if (menu_component.isError()) {
    qCritical() <<
        "uitk::WebContextMenu: Failed to initialize context menu component "
        "because of the following errors: ";
    for (const auto& error : menu_component.errors()) {
      qCritical() << error;
    }
    return false;
  }

  Q_ASSERT(menu_component.isReady());

  QObject* menu = menu_component.beginCreate(engine->rootContext());
  if (!menu) {
    qCritical() <<
        "uitk::WebContextMenu: Failed to create context menu instance";
    return false;
  }

  item_.reset(qobject_cast<QQuickItem*>(menu));
  if (!item_) {
    qCritical() <<
        "uitk::WebContextMenu: Context menu instance is not a QQuickItem";
    delete menu;
    return false;
  }

  item_->setProperty(
      "items",
      OxideUbuntuWebContextMenuPrivate::get(
          menu_.get())->GetItemsAsVariantList());
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

void WebContextMenu::OnStockActionTriggered(const QVariant&) {
  QVariant action_v = sender()->property("__stock_action");
  Q_ASSERT(action_v.isValid());
  unsigned action = action_v.toUInt();
  client_->execCommand(static_cast<WebContextMenuAction>(action));
}

// static
std::unique_ptr<WebContextMenu> WebContextMenu::Create(
    QQmlEngine* engine,
    QQuickItem* parent,
    const WebContextMenuParams& params,
    std::unique_ptr<OxideUbuntuWebContextMenu> menu,
    const std::vector<QObject*>& stock_actions,
    WebContextMenuClient* client,
    bool mobile) {
  std::unique_ptr<WebContextMenu> web_context_menu(
      new WebContextMenu(parent, std::move(menu), stock_actions, client));
  if (!web_context_menu->Init(engine, params, mobile)) {
    return nullptr;
  }

  return std::move(web_context_menu);
}

WebContextMenu::~WebContextMenu() = default;

} // namespace uitk
} // namespace oxide
