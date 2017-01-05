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

#include "oxideubuntuwebcontextmenu.h"
#include "oxideubuntuwebcontextmenu_p.h"

#include <QtDebug>

#include "oxideubuntuwebcontextmenuitem_p.h"

OxideUbuntuWebContextMenuPrivate::OxideUbuntuWebContextMenuPrivate(
    OxideUbuntuWebContextMenu* q)
    : q_ptr(q) {}

// static
int OxideUbuntuWebContextMenuPrivate::items_Count(
    QQmlListProperty<OxideUbuntuWebContextMenuItem>* prop) {
  OxideUbuntuWebContextMenuPrivate* d =
      OxideUbuntuWebContextMenuPrivate::get(
          static_cast<OxideUbuntuWebContextMenu*>(prop->object));
  return d->items_.size();
}

// static
OxideUbuntuWebContextMenuItem* OxideUbuntuWebContextMenuPrivate::items_At(
    QQmlListProperty<OxideUbuntuWebContextMenuItem>* prop,
    int index) {
  OxideUbuntuWebContextMenuPrivate* d =
      OxideUbuntuWebContextMenuPrivate::get(
          static_cast<OxideUbuntuWebContextMenu*>(prop->object));
  return d->items_.at(index);
}

bool OxideUbuntuWebContextMenuPrivate::PrepareToAddItem(
    OxideUbuntuWebContextMenuItem* item) {
  Q_Q(OxideUbuntuWebContextMenu);

  if (!item) {
    qWarning() << "OxideUbuntuWebContextMenu: Cannot add a null item";
    return false;
  }

  OxideUbuntuWebContextMenuItemPrivate* item_d =
      OxideUbuntuWebContextMenuItemPrivate::get(item);

  if (item_d->menu()) {
    qWarning() <<
        "OxideUbuntuWebContextMenu: Cannot add an item that already belongs "
        "to another menu";
    return false;
  }

  Q_ASSERT(item->stockAction() == OxideUbuntuWebContextMenuItem::NoAction);
  Q_ASSERT(item->section() == OxideUbuntuWebContextMenuItem::NoSection);
  Q_ASSERT(q->indexOfItem(item) == -1);

  item_d->set_menu(q);
  item->setParent(q);

  return true;
}

bool OxideUbuntuWebContextMenuPrivate::InsertItemAt(
    int index,
    OxideUbuntuWebContextMenuItem* item) {
  Q_Q(OxideUbuntuWebContextMenu);

  if (!PrepareToAddItem(item)) {
    return false;
  }

  bool was_empty = q->isEmpty();

  items_.insert(index, item);

  if (index > 0 && index < items_.size() - 1 &&
      items_.at(index - 1)->section() == items_.at(index + 1)->section() &&
      items_.at(index - 1)->section() != OxideUbuntuWebContextMenuItem::NoSection) {
    OxideUbuntuWebContextMenuItemPrivate::get(item)->SetSection(
        items_.at(index - 1)->section());
  }

  Q_EMIT q->itemsChanged();

  if (!was_empty) {
    return true;
  }

  Q_EMIT q->isEmptyChanged();
  return true;
}

void OxideUbuntuWebContextMenuPrivate::DiscardCreatedItem(
    OxideUbuntuWebContextMenuItem* item) {
  if (item->action()->parent() == item) {
    item->action()->setParent(nullptr);
  }
  delete item;
}

OxideUbuntuWebContextMenuPrivate::~OxideUbuntuWebContextMenuPrivate() = default;

// static
OxideUbuntuWebContextMenuPrivate* OxideUbuntuWebContextMenuPrivate::get(
    OxideUbuntuWebContextMenu* q) {
  return q->d_func();
}

void OxideUbuntuWebContextMenuPrivate::AppendItem(
    OxideUbuntuWebContextMenuItem* item) {
  Q_Q(OxideUbuntuWebContextMenu);

  if (sections_.find(item->section()) != sections_.end()) {
    Q_ASSERT(!items_.isEmpty());
    Q_ASSERT(items_.back()->section() == item->section());
  }

  OxideUbuntuWebContextMenuItemPrivate* item_d =
      OxideUbuntuWebContextMenuItemPrivate::get(item);
  Q_ASSERT(!item_d->menu());

  sections_.insert(item->section());

  item_d->set_menu(q);
  item->setParent(q);

  items_.push_back(item);
}

QVariantList OxideUbuntuWebContextMenuPrivate::GetItemsAsVariantList() const {
  QVariantList rv;
  for (auto* item : items_) {
    rv.push_back(QVariant::fromValue(item));
  }
  return rv;
}

OxideUbuntuWebContextMenu::OxideUbuntuWebContextMenu(QObject* parent)
    : QObject(parent),
      d_ptr(new OxideUbuntuWebContextMenuPrivate(this)) {
  qRegisterMetaType<OxideUbuntuWebContextMenuItem::Action>();
  qRegisterMetaType<OxideUbuntuWebContextMenuItem::Section>();
}

OxideUbuntuWebContextMenu::~OxideUbuntuWebContextMenu() = default;

bool OxideUbuntuWebContextMenu::isEmpty() const {
  Q_D(const OxideUbuntuWebContextMenu);

  return d->items_.isEmpty();
}

QQmlListProperty<OxideUbuntuWebContextMenuItem>
OxideUbuntuWebContextMenu::items() {
  return QQmlListProperty<OxideUbuntuWebContextMenuItem>(
      this, nullptr,
      OxideUbuntuWebContextMenuPrivate::items_Count,
      OxideUbuntuWebContextMenuPrivate::items_At);
}

int OxideUbuntuWebContextMenu::indexOfItem(
    OxideUbuntuWebContextMenuItem* item) const {
  Q_D(const OxideUbuntuWebContextMenu);

  if (!item) {
    qWarning() << "OxideUbuntuWebContextMenu::indexOfItem: No item specified";
    return -1;
  }

  return d->items_.indexOf(item);
}

int OxideUbuntuWebContextMenu::indexOfStockAction(
    OxideUbuntuWebContextMenuItem::Action action) const {
  Q_D(const OxideUbuntuWebContextMenu);

  if (action == OxideUbuntuWebContextMenuItem::NoAction) {
    qWarning() <<
        "OxideUbuntuWebContextMenu::indexOfStockAction: Must specify an action "
        "other than NoAction";
    return -1;
  }

  for (auto* item : d->items_) {
    if (item->stockAction() == action) {
      return indexOfItem(item);
    }
  }

  return -1;
}

void OxideUbuntuWebContextMenu::appendItem(
    OxideUbuntuWebContextMenuItem* item) {
  Q_D(OxideUbuntuWebContextMenu);

  if (!d->PrepareToAddItem(item)) {
    return;
  }

  bool was_empty = isEmpty();

  d->items_.push_back(item);
  Q_EMIT itemsChanged();

  if (!was_empty) {
    return;
  }

  Q_EMIT isEmptyChanged();
}

int OxideUbuntuWebContextMenu::appendItemToSection(
    OxideUbuntuWebContextMenuItem::Section section,
    OxideUbuntuWebContextMenuItem* item) {
  Q_D(OxideUbuntuWebContextMenu);

  if (d->sections_.find(section) == d->sections_.end()) {
    qWarning() <<
        "OxideUbuntuWebContextMenu::appendItemToSection: The specified section "
        "does not exist in this menu";
    return -1;
  }

  bool found_section = false;
  int index = -1;

  for (int i = 0; i < d->items_.size(); ++i) {
    if (d->items_.at(i)->section() == section) {
      found_section = true;
    } else if (found_section) {
      index = i;
      break;
    }
  }

  Q_ASSERT(found_section);

  if (index == -1) {
    index = d->items_.size();
  }

  if (!d->InsertItemAt(index, item)) {
    return -1;
  }

  OxideUbuntuWebContextMenuItemPrivate::get(item)->SetSection(section);
  return index;
}

void OxideUbuntuWebContextMenu::appendAction(QObject* action) {
  Q_D(OxideUbuntuWebContextMenu);

  if (!action) {
    qWarning() <<
        "OxideUbuntuWebContextMenu::appendAction: No action specified";
    return;
  }

  OxideUbuntuWebContextMenuItem* item =
      OxideUbuntuWebContextMenuItemPrivate::CreateItem(action);
  appendItem(item);
}

int OxideUbuntuWebContextMenu::appendActionToSection(
    OxideUbuntuWebContextMenuItem::Section section,
    QObject* action) {
  Q_D(OxideUbuntuWebContextMenu);

  if (!action) {
    qWarning() <<
        "OxideUbuntuWebContextMenu::appendActionToSection: No action specified";
    return -1;
  }

  OxideUbuntuWebContextMenuItem* item =
      OxideUbuntuWebContextMenuItemPrivate::CreateItem(action);

  int index = appendItemToSection(section, item);
  if (index == -1) {
    d->DiscardCreatedItem(item);
  }

  return index;
}

void OxideUbuntuWebContextMenu::prependItem(
    OxideUbuntuWebContextMenuItem* item) {
  Q_D(OxideUbuntuWebContextMenu);

  if (!d->PrepareToAddItem(item)) {
    return;
  }

  bool was_empty = isEmpty();

  d->items_.push_front(item);
  Q_EMIT itemsChanged();

  if (!was_empty) {
    return;
  }

  Q_EMIT isEmptyChanged();
}

int OxideUbuntuWebContextMenu::prependItemToSection(
    OxideUbuntuWebContextMenuItem::Section section,
    OxideUbuntuWebContextMenuItem* item) {
  Q_D(OxideUbuntuWebContextMenu);

  if (d->sections_.find(section) == d->sections_.end()) {
    qWarning() <<
        "OxideUbuntuWebContextMenu::prependItemToSection: The specified "
        "section does not exist in this menu";
    return -1;
  }

  int index = -1;
  for (int i = 0; i < d->items_.size(); ++i) {
    if (d->items_.at(i)->section() == section) {
      index = i;
      break;
    }
  }

  Q_ASSERT(index != -1);

  if (!d->InsertItemAt(index, item)) {
    return -1;
  }

  OxideUbuntuWebContextMenuItemPrivate::get(item)->SetSection(section);
  return index;
}

void OxideUbuntuWebContextMenu::prependAction(QObject* action) {
  Q_D(OxideUbuntuWebContextMenu);

  if (!action) {
    qWarning() <<
        "OxideUbuntuWebContextMenu::prependAction: No action specified";
    return;
  }

  OxideUbuntuWebContextMenuItem* item =
      OxideUbuntuWebContextMenuItemPrivate::CreateItem(action);
  prependItem(item);
}

int OxideUbuntuWebContextMenu::prependActionToSection(
    OxideUbuntuWebContextMenuItem::Section section,
    QObject* action) {
  Q_D(OxideUbuntuWebContextMenu);

  if (!action) {
    qWarning() <<
        "OxideUbuntuWebContextMenu::prependActionToSection: No action "
        "specified";
    return -1;
  }

  OxideUbuntuWebContextMenuItem* item =
      OxideUbuntuWebContextMenuItemPrivate::CreateItem(action);

  int index = prependItemToSection(section, item);
  if (index == -1) {
    d->DiscardCreatedItem(item);
  }

  return index;
}

void OxideUbuntuWebContextMenu::insertItemAt(
    int index,
    OxideUbuntuWebContextMenuItem* item) {
  Q_D(OxideUbuntuWebContextMenu);

  d->InsertItemAt(index, item);
}

void OxideUbuntuWebContextMenu::insertActionAt(int index,
                                               QObject* action) {
  Q_D(OxideUbuntuWebContextMenu);

  if (!action) {
    qWarning() <<
        "OxideUbuntuWebContextMenu::insertActionAt: No action specified";
    return;
  }

  OxideUbuntuWebContextMenuItem* item =
      OxideUbuntuWebContextMenuItemPrivate::CreateItem(action);
  insertItemAt(index, item);
}

int OxideUbuntuWebContextMenu::insertItemBefore(
    OxideUbuntuWebContextMenuItem::Action before,
    OxideUbuntuWebContextMenuItem* item) {
  Q_D(OxideUbuntuWebContextMenu);

  int index = indexOfStockAction(before);
  if (index == -1) {
    return index;
  }

  if (!d->InsertItemAt(index, item)) {
    return -1;
  }

  return index;
}

int OxideUbuntuWebContextMenu::insertActionBefore(
    OxideUbuntuWebContextMenuItem::Action before,
    QObject* action) {
  Q_D(OxideUbuntuWebContextMenu);

  if (!action) {
    qWarning() <<
        "OxideUbuntuWebContextMenu::insertActionBefore: No action specified";
    return -1;
  }

  OxideUbuntuWebContextMenuItem* item =
      OxideUbuntuWebContextMenuItemPrivate::CreateItem(action);

  int index = insertItemBefore(before, item);
  if (index == -1) {
    d->DiscardCreatedItem(item);
  }

  return index;
}

int OxideUbuntuWebContextMenu::insertItemAfter(
    OxideUbuntuWebContextMenuItem::Action after,
    OxideUbuntuWebContextMenuItem* item) {
  Q_D(OxideUbuntuWebContextMenu);

  int index = indexOfStockAction(after);
  if (index == -1) {
    return index;
  }

  ++index;
  if (!d->InsertItemAt(index, item)) {
    return -1;
  }

  return index;
}

int OxideUbuntuWebContextMenu::insertActionAfter(
    OxideUbuntuWebContextMenuItem::Action after,
    QObject* action) {
  Q_D(OxideUbuntuWebContextMenu);

  if (!action) {
    qWarning() <<
        "OxideUbuntuWebContextMenu::insertActionAfter: No Action specified";
    return -1;
  }

  OxideUbuntuWebContextMenuItem* item =
      OxideUbuntuWebContextMenuItemPrivate::CreateItem(action);

  int index = insertItemAfter(after, item);
  if (index == -1) {
    d->DiscardCreatedItem(item);
  }

  return index;
}

void OxideUbuntuWebContextMenu::removeItem(
    OxideUbuntuWebContextMenuItem* item) {
  Q_D(OxideUbuntuWebContextMenu);

  if (!item) {
    qWarning() << "OxideUbuntuWebContextMenu::removeItem: No item specified";
    return;
  }

  if (!d->items_.removeOne(item)) {
    qWarning() <<
        "OxideUbuntuWebContextMenu::removeItem: Specified item is not part "
        "of this menu";
    return;
  }

  OxideUbuntuWebContextMenuItemPrivate* item_d =
      OxideUbuntuWebContextMenuItemPrivate::get(item);

  Q_ASSERT(item->parent() == this);
  Q_ASSERT(item_d->menu() == this);

  item->setParent(nullptr);
  item_d->set_menu(nullptr);

  Q_EMIT itemsChanged();

  if (!isEmpty()) {
    return;
  }

  Q_EMIT isEmptyChanged();
}

void OxideUbuntuWebContextMenu::removeItemAt(int index) {
  Q_D(OxideUbuntuWebContextMenu);

  if (index < 0 || index >= d->items_.size()) {
    qWarning() << "OxideUbuntuWebContextMenu::removeItemAt: Invalid index";
    return;
  }

  OxideUbuntuWebContextMenuItem* item = d->items_.at(index);
  removeItem(item);

  delete item;
}

int OxideUbuntuWebContextMenu::removeAll() {
  Q_D(OxideUbuntuWebContextMenu);

  int rv = d->items_.size();
  while (d->items_.size() > 0) {
    removeItemAt(0);
  }

  return rv;
}
