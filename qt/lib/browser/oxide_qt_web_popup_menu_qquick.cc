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

#include "oxide_qt_web_popup_menu_qquick.h"

#include <QAbstractListModel>
#include <QLatin1String>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QRect>
#include <QRectF>
#include <QString>
#include <string>

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/gfx/rect.h"

#include "qt/lib/api/oxide_qquick_web_view_p_p.h"
#include "qt/lib/api/public/oxide_qquick_web_view_p.h"

namespace oxide {
namespace qt {

namespace {

class PopupListModel : public QAbstractListModel {
  Q_OBJECT

 public:
  virtual ~PopupListModel() {}
  PopupListModel(const std::vector<content::MenuItem>& items,
                 int selected_item,
                 bool allow_multiple_selection);

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual QVariant data(const QModelIndex& index,
                        int role = Qt::DisplayRole) const;
  virtual QHash<int, QByteArray> roleNames() const { return roles_; };

  Q_INVOKABLE void select(int index);

  bool allowMultiSelect() const { return allow_multi_select_; }
  std::vector<int> selectedIndices() const;

 private:
  enum Roles {
    GroupRole = Qt::UserRole,
    EnabledRole = Qt::UserRole + 1,
    SelectedRole = Qt::UserRole + 2,
    SeparatorRole = Qt::UserRole + 3
  };

  struct Item {
    Item(const content::MenuItem& item, int index,
         const std::string& group, bool add_check) :
        label(QString::fromStdString(base::UTF16ToUTF8(item.label))),
        tooltip(QString::fromStdString(base::UTF16ToUTF8(item.tool_tip))),
        group(QString::fromStdString(group)),
        index(index),
        enabled(item.enabled),
        checked(item.checked || add_check),
        separator(item.type == content::MenuItem::SEPARATOR) {}

    QString label;
    QString tooltip;
    QString group;
    int index;
    bool enabled;
    bool checked;
    bool separator;
  };

  bool allow_multi_select_;
  std::vector<Item> items_;
  int selected_index_;
  QHash<int, QByteArray> roles_;
};

PopupListModel::PopupListModel(
    const std::vector<content::MenuItem>& items,
    int selected_item,
    bool allow_multiple_selection) :
    allow_multi_select_(allow_multiple_selection),
    selected_index_(-1) {
  roles_[Qt::DisplayRole] = "text";
  roles_[Qt::ToolTipRole] = "tooltip";
  roles_[GroupRole] = "group";
  roles_[EnabledRole] = "enabled";
  roles_[SelectedRole] = "selected";
  roles_[SeparatorRole] = "isSeparator";

  // Chromium uses int for indices, so we shouldn't have greater
  // than INT_MAX items. If the number of items is greater than
  // INT_MAX, then we don't run this loop
  if (items.size() > INT_MAX) {
    return;
  }

  std::string current_group;

  int j = 0;
  for (int i = 0; i < static_cast<int>(items.size()); ++i) {
    const content::MenuItem& item = items[i];
    if (item.type == content::MenuItem::GROUP) {
      current_group = base::UTF16ToUTF8(item.label);
      continue;
    }

    if (!allow_multiple_selection && selected_item == i &&
        selected_item >= 0) {
      selected_index_ = j;
    }

    items_.push_back(Item(item, i, current_group, selected_index_ == j));
    ++j;
  }
}

int PopupListModel::rowCount(const QModelIndex& parent) const {
  return items_.size();
}

QVariant PopupListModel::data(const QModelIndex& index, int role) const {
  // The cast here is ok because the number of items_ is never
  // greater than INT_MAX
  if (!index.isValid() || index.row() < 0 ||
      index.row() > static_cast<int>(items_.size())) {
    return QVariant();
  }

  const Item& item = items_[index.row()];
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
    NOTREACHED();
    return QVariant();
  }
}

void PopupListModel::select(int index) {
  // The cast here is ok because the number of items_ is never
  // greater than INT_MAX
  if (index < 0 || index > static_cast<int>(items_.size())) {
    return;
  }

  Item& item = items_[index];
  if (!item.enabled) {
    return;
  }

  if (allow_multi_select_) {
    item.checked = !item.checked;
  } else {
    if (index == selected_index_) {
      return;
    }

    DCHECK(!item.checked);

    if (selected_index_ != -1) {
      Item& old_item = items_[selected_index_];
      DCHECK(old_item.checked);

      old_item.checked = false;
      emit dataChanged(this->index(selected_index_),
                       this->index(selected_index_));
    }

    item.checked = true;
    selected_index_ = index;
  }

  emit dataChanged(this->index(index), this->index(index));
}

std::vector<int> PopupListModel::selectedIndices() const {
  std::vector<int> rv;
  for (size_t i = 0; i < items_.size(); ++i) {
    const Item& item = items_[i];
    if (item.checked) {
      rv.push_back(item.index);
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
  PopupMenuContext(WebPopupMenuQQuick* popup_menu,
                   const gfx::Rect& bounds,
                   const std::vector<content::MenuItem>& items,
                   int selected_item,
                   bool allow_multiple_selection);

  QRectF elementRect() const { return element_rect_; }
  QObject* items() { return &items_; }
  bool allowMultiSelect() const { return items_.allowMultiSelect(); }

  Q_INVOKABLE void accept();
  Q_INVOKABLE void cancel();

 private:
  WebPopupMenuQQuick* popup_menu_;
  QRectF element_rect_;
  PopupListModel items_;
};

PopupMenuContext::PopupMenuContext(WebPopupMenuQQuick* popup_menu,
                                   const gfx::Rect& bounds,
                                   const std::vector<content::MenuItem>& items,
                                   int selected_item,
                                   bool allow_multiple_selection) :
    popup_menu_(popup_menu),
    element_rect_(QRect(bounds.x(), bounds.y(), bounds.width(), 
                        bounds.height())),
    items_(items, selected_item, allow_multiple_selection) {}

void PopupMenuContext::accept() {
  std::vector<int> indices = items_.selectedIndices();
  DCHECK(items_.allowMultiSelect() || indices.size() <= 1);
  popup_menu_->SelectItems(indices);
}

void PopupMenuContext::cancel() {
  popup_menu_->Cancel();
}

} // namespace

WebPopupMenuQQuick::WebPopupMenuQQuick(OxideQQuickWebView* view,
                                       content::WebContents* web_contents) :
    oxide::WebPopupMenu(web_contents),
    view_(view) {

}

void WebPopupMenuQQuick::Show(const gfx::Rect& bounds,
                              const std::vector<content::MenuItem>& items,
                              int selected_item,
                              bool allow_multiple_selection) {
  QQmlComponent* component = view_->popupMenu();
  if (!component) {
    Cancel();
    return;
  }

  PopupMenuContext* contextObject =
      new PopupMenuContext(this, bounds, items, selected_item,
                           allow_multiple_selection);

  QQmlContext* baseContext = component->creationContext();
  if (!baseContext) {
    baseContext = QQmlEngine::contextForObject(view_);
  }
  context_.reset(new QQmlContext(baseContext));

  context_->setContextProperty(QLatin1String("model"), contextObject);
  context_->setContextObject(contextObject);
  contextObject->setParent(context_.get());

  popup_menu_.reset(
      qobject_cast<QQuickItem *>(component->beginCreate(context_.get())));
  if (!popup_menu_) {
    Cancel();
    return;
  }

  QQuickWebViewPrivate::get(view_)->addAttachedPropertyTo(popup_menu_.get());
  popup_menu_->setParentItem(view_);

  component->completeCreate();
}

} // namespace qt
} // namespace oxide

#include "oxide_qt_web_popup_menu_qquick.moc"
