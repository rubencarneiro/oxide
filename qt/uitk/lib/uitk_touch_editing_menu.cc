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

#include "uitk_touch_editing_menu.h"

#include <cmath>

#include <QLatin1String>
#include <QObject>
#include <QPointF>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QQmlExpression>
#include <QQuickItem>
#include <QQuickView>
#include <QSizeF>
#include <QString>
#include <QtDebug>

#include "qt/core/glue/touch_editing_menu_client.h"
#include "qt/core/glue/web_context_menu_actions.h"

namespace oxide {
namespace uitk {

using qt::EditCapabilityFlags;
using qt::TouchEditingMenuClient;
using qt::WebContextMenuAction;

namespace {

WebContextMenuAction EditingCommandFromAction(const QString& action) {
  if (action == QLatin1String("selectall")) {
    return WebContextMenuAction::SelectAll;
  } else if (action == QLatin1String("cut")) {
    return WebContextMenuAction::Cut;
  } else if (action == QLatin1String("copy")) {
    return WebContextMenuAction::Copy;
  } else if (action == QLatin1String("paste")) {
    return WebContextMenuAction::Paste;
  }

  Q_UNREACHABLE();
}

QQuickItem* GetRootItem(QQuickItem* item) {
  QQuickView* view = qobject_cast<QQuickView*>(item->window());
  if (view) {
    return view->rootObject();
  }

  QQuickItem* root = item->window()->contentItem();
  return root->childItems()[0];
}

}

void TouchEditingMenu::OnActionTriggered(const QString& action) {
  client_->ExecuteCommand(EditingCommandFromAction(action));
}

void TouchEditingMenu::OnVisibleChanged() {
  if (!inside_hide_ && !item_->isVisible()) {
    client_->Close();
  }
}

void TouchEditingMenu::OnResize() {
  client_->WasResized();
}

TouchEditingMenu::TouchEditingMenu(QQuickItem* parent,
                                   TouchEditingMenuClient* client)
    : parent_(parent),
      client_(client) {}

bool TouchEditingMenu::Init(EditCapabilityFlags edit_flags) {
  QQmlEngine* engine = qmlEngine(parent_.data());
  if (!engine) {
    qWarning() <<
        "uitk::TouchEditingMenu: Failed to open touch editing menu - cannot "
        "determine QQmlEngine for parent item";
    return false;
  }

  if (!parent_->window()) {
    qWarning() <<
        "uitk::TouchEditingMenu: Failed to open touch editing menu - item is "
        "not in a window";
    return false;
  }

  QQmlComponent component(engine);
  component.loadUrl(QUrl("qrc:///TouchEditingMenu.qml"));

  if (component.isError()) {
    qCritical() <<
        "uitk::TouchEditingMenu: Failed to initialize touch editing menu "
        "component because of the following errors: ";
    for (const auto& error : component.errors()) {
      qCritical() << error;
    }
    return false;
  }

  Q_ASSERT(component.isReady());

  QObject* menu = component.beginCreate(engine->rootContext());
  if (!menu) {
    qCritical() <<
        "uitk::TouchEditingMenu: Failed to create touch editing menu instance";
    return false;
  }

  item_.reset(qobject_cast<QQuickItem*>(menu));
  if (!item_) {
    qCritical() <<
        "uitk::TouchEditingMenu: Touch editing menu instance is not a "
        "QQuickItem";
    delete menu;
    return false;
  }

  item_->setZ(100);
  item_->setProperty("editFlags", QVariant(edit_flags));
  item_->setParentItem(GetRootItem(parent_.data()));

  connect(item_.get(), SIGNAL(actionTriggered(const QString&)),
          SLOT(OnActionTriggered(const QString&)));
  connect(item_.get(), &QQuickItem::visibleChanged,
          this, &TouchEditingMenu::OnVisibleChanged);
  connect(item_.get(), &QQuickItem::widthChanged,
          this, &TouchEditingMenu::OnResize);

  component.completeCreate();
}

void TouchEditingMenu::Show() {
  item_->setVisible(true);
}

void TouchEditingMenu::Hide() {
  inside_hide_ = true;
  item_->setVisible(false);
  inside_hide_ = false;

}

QSize TouchEditingMenu::GetSizeIncludingMargin() const {
  QSize size(std::ceil(item_->width()), std::ceil(item_->height()));

  QQmlExpression expression(qmlContext(item_.get()),
                            item_.get(),
                            "units.gu(1)");
  int margin = expression.evaluate().toInt();

  return size + QSize(margin, margin);
}

void TouchEditingMenu::SetOrigin(const QPointF& origin) {
  QPointF origin_in_scene = parent_->mapToScene(origin);
  item_->setX(origin_in_scene.x());
  item_->setY(origin_in_scene.y());
}

// static
std::unique_ptr<TouchEditingMenu> TouchEditingMenu::Create(
    QQuickItem* parent,
    EditCapabilityFlags edit_flags,
    TouchEditingMenuClient* client) {
  std::unique_ptr<TouchEditingMenu> menu(new TouchEditingMenu(parent, client));
  if (!menu->Init(edit_flags)) {
    return nullptr;
  }
  return menu;
}

TouchEditingMenu::~TouchEditingMenu() = default;

} // namespace uitk
} // namespace oxide
