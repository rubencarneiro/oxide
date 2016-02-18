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

#ifndef _OXIDE_QQUICK_CONTENTS_VIEW_H_
#define _OXIDE_QQUICK_CONTENTS_VIEW_H_

#include <QPointer>
#include <QtGlobal>

#include "qt/core/glue/oxide_qt_contents_view_proxy_client.h"

QT_BEGIN_NAMESPACE
class QQmlComponent;
class QQuickItem;
QT_END_NAMESPACE

namespace oxide {
namespace qquick {

class ContentsView : public oxide::qt::ContentsViewProxyClient {
 public:
  ContentsView(QQuickItem* item);
  ~ContentsView() override;

  // XXX(chrisccoulson): Remove these UI component accessors from here in
  //  future. Instead, we should have an auxilliary UI client class. There'll
  //  be a WebView impl of this for the custom UI component APIs, and also an
  //  Ubuntu UI Toolkit impl
  QQmlComponent* contextMenu() const { return context_menu_; }
  void setContextMenu(QQmlComponent* context_menu) {
    context_menu_ = context_menu;
  }

  QQmlComponent* popupMenu() const { return popup_menu_; }
  void setPopupMenu(QQmlComponent* popup_menu) {
    popup_menu_ = popup_menu;
  }

 private:
  // oxide::qt::ContentsViewProxyClient implementation
  QScreen* GetScreen() const override;
  QRect GetBoundsPix() const override;
  oxide::qt::WebContextMenuProxy* CreateWebContextMenu(
      oxide::qt::WebContextMenuProxyClient* client) override;
  oxide::qt::WebPopupMenuProxy* CreateWebPopupMenu(
      oxide::qt::WebPopupMenuProxyClient* client) override;

  QPointer<QQuickItem> item_;

  QPointer<QQmlComponent> context_menu_;
  QPointer<QQmlComponent> popup_menu_;

  Q_DISABLE_COPY(ContentsView)
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QQUICK_CONTENTS_VIEW_H_
