// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#include "oxide_qquick_web_popup_menu_delegate.h"

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

#include "qt/quick/api/oxideqquickwebview_p.h"
#include "qt/quick/api/oxideqquickwebview_p_p.h"

namespace oxide {
namespace qquick {

namespace {

class PopupListModel : public QAbstractListModel {
  Q_OBJECT

 public:
  virtual ~PopupListModel() {}
  PopupListModel(QList<oxide::qt::MenuItem>& items,
                 bool allow_multiple_selection);

  int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_FINAL;
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const Q_DECL_FINAL;
  QHash<int, QByteArray> roleNames() const Q_DECL_FINAL { return roles_; };

  Q_INVOKABLE void select(int index);

  bool allowMultiSelect() const { return allow_multi_select_; }
  QList<int> selectedIndices() const;

 private:
  enum Roles {
    GroupRole = Qt::UserRole,
    EnabledRole = Qt::UserRole + 1,
    SelectedRole = Qt::UserRole + 2,
    SeparatorRole = Qt::UserRole + 3
  };

  bool allow_multi_select_;
  QList<oxide::qt::MenuItem> items_;
  int selected_index_;
  QHash<int, QByteArray> roles_;
};

PopupListModel::PopupListModel(QList<oxide::qt::MenuItem>& items,
                               bool allow_multiple_selection) :
    allow_multi_select_(allow_multiple_selection),
    selected_index_(-1) {
  items_.swap(items);

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

QList<int> PopupListModel::selectedIndices() const {
  QList<int> rv;
  for (int i = 0; i < items_.size(); ++i) {
    const oxide::qt::MenuItem& item = items_[i];
    if (item.checked) {
      rv.append(item.index);
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
  PopupMenuContext(WebPopupMenuDelegate* delegate,
                   const QRect& bounds,
                   QList<oxide::qt::MenuItem>& items,
                   bool allow_multiple_selection);

  QRectF elementRect() const { return element_rect_; }
  QObject* items() { return &items_; }
  bool allowMultiSelect() const { return items_.allowMultiSelect(); }

  Q_INVOKABLE void accept();
  Q_INVOKABLE void cancel();

 private:
  WebPopupMenuDelegate* delegate_;
  QRectF element_rect_;
  PopupListModel items_;
};

PopupMenuContext::PopupMenuContext(WebPopupMenuDelegate* delegate,
                                   const QRect& bounds,
                                   QList<oxide::qt::MenuItem>& items,
                                   bool allow_multiple_selection) :
    delegate_(delegate),
    element_rect_(bounds),
    items_(items, allow_multiple_selection) {}

void PopupMenuContext::accept() {
  QList<int> indices = items_.selectedIndices();
  Q_ASSERT(items_.allowMultiSelect() || indices.size() <= 1);
  delegate_->SelectItems(indices);
}

void PopupMenuContext::cancel() {
  delegate_->Cancel();
}

} // namespace

WebPopupMenuDelegate::WebPopupMenuDelegate(OxideQQuickWebView* webview) :
    web_view_(webview),
    popup_component_(webview->popupMenu()) {}

void WebPopupMenuDelegate::Show(const QRect& bounds,
                                QList<oxide::qt::MenuItem>& items,
                                bool allow_multiple_selection) {
  if (!popup_component_) {
    qWarning() << "Content requested a popup menu, but the application hasn't provided one";
    Cancel();
    return;
  }

  PopupMenuContext* contextObject =
      new PopupMenuContext(this, bounds, items, allow_multiple_selection);

  QQmlContext* baseContext = popup_component_->creationContext();
  if (!baseContext) {
    baseContext = QQmlEngine::contextForObject(web_view_);
  }
  popup_context_.reset(new QQmlContext(baseContext));

  popup_context_->setContextProperty(QLatin1String("model"), contextObject);
  popup_context_->setContextObject(contextObject);
  contextObject->setParent(popup_context_.data());

  popup_item_.reset(qobject_cast<QQuickItem *>(
      popup_component_->beginCreate(popup_context_.data())));
  if (!popup_item_) {
    qWarning() << "Failed to create popup";
    Cancel();
    return;
  }

  OxideQQuickWebViewPrivate::get(web_view_)->addAttachedPropertyTo(
      popup_item_.data());
  popup_item_->setParentItem(web_view_);

  popup_component_->completeCreate();
}

} // namespace qquick
} // namespace oxide

#include "oxide_qquick_web_popup_menu_delegate.moc"
