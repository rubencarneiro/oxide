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

#include "oxideubuntuwebcontextmenuitem.h"
#include "oxideubuntuwebcontextmenuitem_p.h"

#include <QtDebug>

#include "oxideubuntuwebcontextmenu.h"

OxideUbuntuWebContextMenuItemPrivate
    ::OxideUbuntuWebContextMenuItemPrivate() = default;

OxideUbuntuWebContextMenuItemPrivate
    ::~OxideUbuntuWebContextMenuItemPrivate() = default;

// static
OxideUbuntuWebContextMenuItemPrivate* OxideUbuntuWebContextMenuItemPrivate::get(
    OxideUbuntuWebContextMenuItem* q) {
  return q->d_func();
}

// static
OxideUbuntuWebContextMenuItem*
OxideUbuntuWebContextMenuItemPrivate::CreateStockItem(
    OxideUbuntuWebContextMenuItem::Action stock_action,
    OxideUbuntuWebContextMenuItem::Section section,
    QObject* action) {
  OxideUbuntuWebContextMenuItemPrivate* d =
      new OxideUbuntuWebContextMenuItemPrivate();
  d->stock_action_ = stock_action;
  d->section_ = section;
  d->action_ = action;

  OxideUbuntuWebContextMenuItem* item = new OxideUbuntuWebContextMenuItem(*d);
  action->setParent(item);

  return item;
}

// static
OxideUbuntuWebContextMenuItem* OxideUbuntuWebContextMenuItemPrivate::CreateItem(
    QObject* action) {
  OxideUbuntuWebContextMenuItemPrivate* d =
      new OxideUbuntuWebContextMenuItemPrivate();
  d->action_ = action;

  OxideUbuntuWebContextMenuItem* item = new OxideUbuntuWebContextMenuItem(*d);
  if (!action->parent()) {
    action->setParent(item);
  }

  return item;
}

void OxideUbuntuWebContextMenuItemPrivate::SetSection(
    OxideUbuntuWebContextMenuItem::Section section) {
  Q_Q(OxideUbuntuWebContextMenuItem);

  section_ = section;
  Q_EMIT q->sectionChanged();
}

OxideUbuntuWebContextMenuItem::OxideUbuntuWebContextMenuItem(
    OxideUbuntuWebContextMenuItemPrivate& dd,
    QObject* parent)
    : QObject(parent),
      d_ptr(&dd) {
  Q_D(OxideUbuntuWebContextMenuItem);

  d->q_ptr = this;
}

OxideUbuntuWebContextMenuItem::OxideUbuntuWebContextMenuItem(QObject* parent)
    : OxideUbuntuWebContextMenuItem(*new OxideUbuntuWebContextMenuItemPrivate(),
                                    parent) {}

OxideUbuntuWebContextMenuItem::~OxideUbuntuWebContextMenuItem() {
  Q_D(OxideUbuntuWebContextMenuItem);

  if (d->menu_) {
    d->menu_->removeItem(this);
  }
}

OxideUbuntuWebContextMenuItem::Action
OxideUbuntuWebContextMenuItem::stockAction() const {
  Q_D(const OxideUbuntuWebContextMenuItem);

  return d->stock_action_;
}

OxideUbuntuWebContextMenuItem::Section
OxideUbuntuWebContextMenuItem::section() const {
  Q_D(const OxideUbuntuWebContextMenuItem);

  return d->section_;
}

QObject* OxideUbuntuWebContextMenuItem::action() const {
  Q_D(const OxideUbuntuWebContextMenuItem);

  return d->action_.data();
}

void OxideUbuntuWebContextMenuItem::setAction(QObject* action) {
  Q_D(OxideUbuntuWebContextMenuItem);

  if (d->stock_action_ != NoAction) {
    qWarning() <<
        "OxideUbuntuWebContextMenuItem::setAction: Cannot change the action "
        "associated with a stock menu item";
    return;
  }

  if (action == d->action_) {
    return;
  }

  if (action && !action->inherits("UCAction")) {
    qWarning() <<
        "OxideUbuntuWebContextMenuItem::setAction: The specified action has an "
        "unrecognized type";
    return;
  }

  if (d->action_ && d->action_->parent() == this) {
    delete d->action_;
  }

  d->action_ = action;

  if (d->action_ && !d->action_->parent()) {
    d->action_->setParent(this);
  }

  Q_EMIT actionChanged();
}
