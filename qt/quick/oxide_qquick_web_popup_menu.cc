// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include "oxide_qquick_web_popup_menu.h"

#include <QAbstractListModel>
#include <QLatin1String>
#include <QList>
#include <QObject>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QRect>
#include <QRectF>
#include <QtDebug>

#include "qt/core/glue/menu_item.h"
#include "qt/core/glue/oxide_qt_web_popup_menu_proxy_client.h"
#include "qt/quick/api/oxideqquickwebview.h"
#include "qt/quick/api/oxideqquickwebview_p.h"

namespace oxide {
namespace qquick {

namespace {

class PopupListModel : public QAbstractListModel {
  Q_OBJECT

 public:
  virtual ~PopupListModel() {}
  PopupListModel(const std::vector<oxide::qt::MenuItem>& items,
                 bool allow_multiple_selection);

  int rowCount(const QModelIndex& parent = QModelIndex()) const final;
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const final;
  QHash<int, QByteArray> roleNames() const final { return roles_; };

  Q_INVOKABLE void select(int index);

  bool allowMultiSelect() const { return allow_multi_select_; }
  QList<unsigned> selectedIndices() const;

 private:
  enum Roles {
    GroupRole = Qt::UserRole,
    EnabledRole = Qt::UserRole + 1,
    SelectedRole = Qt::UserRole + 2,
    SeparatorRole = Qt::UserRole + 3
  };

  bool allow_multi_select_;
  std::vector<oxide::qt::MenuItem> items_;
  int selected_index_;
  QHash<int, QByteArray> roles_;
};

PopupListModel::PopupListModel(const std::vector<oxide::qt::MenuItem>& items,
                               bool allow_multiple_selection) :
    allow_multi_select_(allow_multiple_selection),
    selected_index_(-1) {
  items_ = items;

  if (!allow_multi_select_) {
    for (int i = 0; i < items_.size(); ++i) {
      if (items_.at(i).checked) {
        selected_index_ = i;
        break;
      }
    }
  }

  // XXX: This should be global
  roles_[Qt::DisplayRole] = "text";
  roles_[Qt::ToolTipRole] = "tooltip";
  roles_[GroupRole] = "group";
  roles_[EnabledRole] = "enabled";
  roles_[SelectedRole] = "selected";
  roles_[SeparatorRole] = "isSeparator";
}

int PopupListModel::rowCount(const QModelIndex& parent) const {
  Q_UNUSED(parent);
  return items_.size();
}

QVariant PopupListModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() > items_.size()) {
    return QVariant();
  }

  const oxide::qt::MenuItem& item = items_[index.row()];
  if (item.separator) {
    if (role == SeparatorRole) {
      return true;
    }

    return QVariant();
  }

  switch (role) {
  case Qt::DisplayRole:
    return item.label;
  case Qt::ToolTipRole:
    return item.tooltip;
  case GroupRole:
    return item.group;
  case EnabledRole:
    return item.enabled;
  case SelectedRole:
    return item.checked;
  case SeparatorRole:
    return false;
  default:
    Q_ASSERT(0);
    return QVariant();
  }
}

void PopupListModel::select(int index) {
  if (index < 0 || index > items_.size()) {
    return;
  }

  oxide::qt::MenuItem& item = items_[index];
  if (!item.enabled) {
    return;
  }

  if (allow_multi_select_) {
    item.checked = !item.checked;
  } else {
    if (index == selected_index_) {
      return;
    }

    Q_ASSERT(!item.checked);

    if (selected_index_ != -1) {
      oxide::qt::MenuItem& old_item = items_[selected_index_];
      Q_ASSERT(old_item.checked);

      old_item.checked = false;
      emit dataChanged(this->index(selected_index_),
                       this->index(selected_index_));
    }

    item.checked = true;
    selected_index_ = index;
  }

  emit dataChanged(this->index(index), this->index(index));
}

QList<unsigned> PopupListModel::selectedIndices() const {
  QList<unsigned> rv;
  for (int i = 0; i < items_.size(); ++i) {
    const oxide::qt::MenuItem& item = items_[i];
    if (item.checked) {
      rv.append(item.action);
    }
  }

  return rv;
}

class PopupMenuContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(QRectF elementRect READ elementRect CONSTANT FINAL)
  Q_PROPERTY(QObject* items READ items CONSTANT FINAL)
  Q_PROPERTY(bool allowMultiSelect READ allowMultiSelect CONSTANT FINAL)

 public:
  virtual ~PopupMenuContext() {}
  PopupMenuContext(oxide::qt::WebPopupMenuProxyClient* client,
                   const QRect& bounds,
                   const std::vector<oxide::qt::MenuItem>& items,
                   bool allow_multiple_selection);

  QRectF elementRect() const { return element_rect_; }
  QObject* items() { return &items_; }
  bool allowMultiSelect() const { return items_.allowMultiSelect(); }

  Q_INVOKABLE void accept();
  Q_INVOKABLE void cancel();

 private:
  oxide::qt::WebPopupMenuProxyClient* client_;
  QRectF element_rect_;
  PopupListModel items_;
};

PopupMenuContext::PopupMenuContext(oxide::qt::WebPopupMenuProxyClient* client,
                                   const QRect& bounds,
                                   const std::vector<oxide::qt::MenuItem>& items,
                                   bool allow_multiple_selection)
    : client_(client),
      element_rect_(bounds),
      items_(items, allow_multiple_selection) {}

void PopupMenuContext::accept() {
  QList<unsigned> indices = items_.selectedIndices();
  Q_ASSERT(items_.allowMultiSelect() || indices.size() <= 1);
  client_->selectItems(indices);
}

void PopupMenuContext::cancel() {
  client_->cancel();
}

} // namespace

void WebPopupMenu::Show(const QRect& bounds) {
  if (!parent_) {
    qWarning() << "WebPopupMenu::Show: Can't show after the view has gone";
    client_->cancel();
    return;
  }

  if (!component_) {
    qWarning() <<
        "WebPopupMenu::Show: Content requested a popup menu, but the "
        "application hasn't provided one";
    client_->cancel();
    return;
  }

  PopupMenuContext* contextObject =
      new PopupMenuContext(client_, bounds, items_, allow_multiple_selection_);

  QQmlContext* baseContext = component_->creationContext();
  if (!baseContext) {
    baseContext = QQmlEngine::contextForObject(parent_);
  }
  popup_context_.reset(new QQmlContext(baseContext));

  popup_context_->setContextProperty(QLatin1String("model"), contextObject);
  popup_context_->setContextObject(contextObject);
  contextObject->setParent(popup_context_.data());

  popup_item_.reset(qobject_cast<QQuickItem *>(
      component_->beginCreate(popup_context_.data())));
  if (!popup_item_) {
    qWarning() <<
        "WebPopupMenu::Show: Failed to create instance of Qml popup component";
    client_->cancel();
    return;
  }

  // This is a bit hacky - not sure what we'll do here if we introduce
  // other items that can use a ContentView and which support custom UI
  // components
  OxideQQuickWebView* web_view = qobject_cast<OxideQQuickWebView*>(parent_);
  if (web_view) {
    OxideQQuickWebViewPrivate::get(web_view)
        ->addAttachedPropertyTo(popup_item_.data());
  }
  popup_item_->setParentItem(parent_);

  component_->completeCreate();
}

void WebPopupMenu::Hide() {
  if (!popup_item_) {
    return;
  }
  popup_item_->setVisible(false);
}

WebPopupMenu::WebPopupMenu(QQuickItem* parent,
                           QQmlComponent* component,
                           const std::vector<oxide::qt::MenuItem>& items,
                           bool allow_multiple_selection,
                           oxide::qt::WebPopupMenuProxyClient* client)
    : items_(items),
      allow_multiple_selection_(allow_multiple_selection),
      client_(client),
      parent_(parent),
      component_(component) {}

WebPopupMenu::~WebPopupMenu() {}

} // namespace qquick
} // namespace oxide

#include "oxide_qquick_web_popup_menu.moc"
