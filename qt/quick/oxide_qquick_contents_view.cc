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

#include "oxide_qquick_contents_view.h"

#include <QQuickItem>
#include <QQuickWindow>

#include "oxide_qquick_web_context_menu.h"
#include "oxide_qquick_web_popup_menu.h"

namespace oxide {
namespace qquick {

QScreen* ContentsView::GetScreen() const {
  if (!item_->window()) {
    return nullptr;
  }

  return item_->window()->screen();
}

QRect ContentsView::GetBoundsPix() const {
  if (!item_->window()) {
    return QRect();
  }

  QPointF pos(item_->mapToScene(QPointF(0, 0)) + item_->window()->position());

  return QRect(qRound(pos.x()), qRound(pos.y()),
               qRound(item_->width()), qRound(item_->height()));
}

oxide::qt::WebContextMenuProxy* ContentsView::CreateWebContextMenu(
    oxide::qt::WebContextMenuProxyClient* client) {
  return new WebContextMenu(item_, context_menu_, client);
}

oxide::qt::WebPopupMenuProxy* ContentsView::CreateWebPopupMenu(
    oxide::qt::WebPopupMenuProxyClient* client) {
  return new WebPopupMenu(item_, popup_menu_, client);
}

ContentsView::ContentsView(QQuickItem* item)
    : item_(item) {}

ContentsView::~ContentsView() {}

} // namespace qquick
} // namespace oxide
