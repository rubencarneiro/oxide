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

#include "uitk_web_popup_menu.h"

#include <QAbstractListModel>
#include <QList>
#include <QMetaObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickItem>
#include <QtDebug>
#include <QVariant>
#include <QVector>

#include "qt/core/glue/web_popup_menu_client.h"

namespace oxide {
namespace uitk {

using qt::MenuItem;

class WebPopupMenu::Model : public QAbstractListModel {
 public:
  Model(const std::vector<MenuItem>& items);
  ~Model() override;

  enum Role {
    RoleLabel = Qt::UserRole,
    RoleGroup,
    RoleEnabled,
    RoleChecked,

    RoleAction
  };

  // QAbstractItemModel implementation
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

 private:
  struct Item {
    QString label;
    QString tooltip;
    QString group;
    bool enabled = true;
    bool checked = false;

    unsigned action = 0;
  };

  QVector<Item> items_;
};

WebPopupMenu::Model::Model(const std::vector<MenuItem>& items) {
  QString current_group;
  for (const auto& item : items) {
    if (item.type == MenuItem::Type::Group) {
      current_group = item.label;
      continue;
    }

    Q_ASSERT(item.type == MenuItem::Type::Option);

    Item i;
    i.label = item.label;
    i.tooltip = item.tooltip;
    i.group = current_group;
    i.enabled = item.enabled;
    i.checked = item.checked;
    i.action = item.action;

    items_.push_back(i);
  }
}

WebPopupMenu::Model::~Model() = default;

int WebPopupMenu::Model::rowCount(const QModelIndex& parent) const {
  return items_.size();
}

QVariant WebPopupMenu::Model::data(const QModelIndex& index, int role) const {
  Q_ASSERT(index.isValid());
  Q_ASSERT(index.row() >= 0 && index.row() < items_.size());
  Q_ASSERT(index.column() == 0);
  Q_ASSERT(!index.parent().isValid());

  const Item& item = items_[index.row()];

  switch (role) {
    case RoleLabel:
      return item.label;
    case RoleGroup:
      return item.group;
    case RoleEnabled:
      return item.enabled;
    case RoleChecked:
      return item.checked;
    case RoleAction:
      return item.action;
    default:
      Q_ASSERT(false);
      return QVariant();
  }
}

QHash<int, QByteArray> WebPopupMenu::Model::roleNames() const {
  static QHash<int, QByteArray> roles;

  if (roles.size() > 0) {
    return roles;
  }

  roles[RoleLabel] = "text";
  roles[RoleGroup] = "group";
  roles[RoleEnabled] = "enabled";
  roles[RoleChecked] = "checked";

  return roles;
}

void WebPopupMenu::Show() {
  QMetaObject::invokeMethod(item_.get(), "show");
}

void WebPopupMenu::Hide() {
  QMetaObject::invokeMethod(item_.get(), "hide");
}

WebPopupMenu::WebPopupMenu(const std::vector<MenuItem>& items,
                           const QRect& bounds,
                           qt::WebPopupMenuClient* client)
    : model_(new Model(items)),
      bounds_(bounds),
      client_(client) {}

bool WebPopupMenu::Init(QQuickItem* parent) {
  QQmlEngine* engine = qmlEngine(parent);
  if (!engine) {
    qWarning() <<
        "uitk::WebPopupMenu: Failed to create popup menu - cannot determine "
        "QQmlEngine for parent item";
    return false;
  }

  QQmlComponent component(engine);
  component.loadUrl(QUrl("qrc:///WebPopupMenu.qml"));

  if (component.isError()) {
    qCritical() <<
        "uitk::WebPopupMenu: Failed to initialize popup menu component "
        "because of the following errors: ";
    for (const auto& error : component.errors()) {
      qCritical() << error;
    }
    return false;
  }

  Q_ASSERT(component.isReady());

  QObject* menu = component.beginCreate(engine->rootContext());
  if (!menu) {
    qCritical() <<
        "uitk::WebPopupMenu: Failed to create popup menu instance";
    return false;
  }

  item_.reset(qobject_cast<QQuickItem*>(menu));
  if (!item_) {
    qCritical() <<
        "uitk::WebPopupMenu: Popup menu instance is not a QQuickItem";
    delete menu;
    return false;
  }

  item_->setProperty("model", QVariant::fromValue(model_.get()));
  item_->setParentItem(parent);

  component.completeCreate();

  connect(item_.get(), &QQuickItem::visibleChanged,
          this, &WebPopupMenu::OnVisibleChanged);
  connect(item_.get(), SIGNAL(selectedItem(int)),
          this, SLOT(OnItemSelected(int)));

  return true;
}

void WebPopupMenu::OnVisibleChanged() {
  if (!item_->isVisible()) {
    client_->cancel();
  }
}

void WebPopupMenu::OnItemSelected(int index) {
  unsigned action =
      model_->data(model_->index(index, 0), Model::RoleAction).toUInt();
  QList<unsigned> items{action};
  client_->selectItems(items);
}

// static
std::unique_ptr<WebPopupMenu> WebPopupMenu::Create(
    QQuickItem* parent,
    const std::vector<MenuItem>& items,
    const QRect& bounds,
    qt::WebPopupMenuClient* client) {
  std::unique_ptr<WebPopupMenu> menu(new WebPopupMenu(items, bounds, client));
  if (!menu->Init(parent)) {
    return nullptr;
  }

  return std::move(menu);
}

WebPopupMenu::~WebPopupMenu() = default;

} // namespace uitk
} // namespace oxide
