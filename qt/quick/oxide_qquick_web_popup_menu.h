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

#ifndef _OXIDE_QT_QUICK_WEB_POPUP_MENU_H_
#define _OXIDE_QT_QUICK_WEB_POPUP_MENU_H_

#include <QPointer>
#include <QScopedPointer>

#include "qt/core/glue/oxide_qt_web_popup_menu_proxy.h"

QT_BEGIN_NAMESPACE
class QQmlComponent;
class QQmlContext;
class QQuickItem;
QT_END_NAMESPACE

namespace oxide {

namespace qt {
class WebPopupMenuProxyClient;
}

namespace qquick {

class WebPopupMenu : public oxide::qt::WebPopupMenuProxy {
 public:
  WebPopupMenu(QQuickItem* parent,
               QQmlComponent* component,
               oxide::qt::WebPopupMenuProxyClient* client);
  ~WebPopupMenu() override;

 private:
  // oxide::qt::WebPopupMenuProxy implementation
  void Show(const QRect& bounds,
            const QList<oxide::qt::MenuItem>& items,
            bool allow_multiple_selection) override;
  void Hide() override;

  oxide::qt::WebPopupMenuProxyClient* client_;
  QPointer<QQuickItem> parent_;
  QPointer<QQmlComponent> component_;
  QScopedPointer<QQuickItem> popup_item_;
  QScopedPointer<QQmlContext> popup_context_;
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QT_QUICK_WEB_POPUP_MENU_H_
