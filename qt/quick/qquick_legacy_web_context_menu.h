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

#ifndef _OXIDE_QT_QUICK_LEGACY_WEB_CONTEXT_MENU_H_
#define _OXIDE_QT_QUICK_LEGACY_WEB_CONTEXT_MENU_H_

#include <memory>

#include <QPointer>

#include "qt/core/glue/web_context_menu.h"
#include "qt/core/glue/web_context_menu_params.h"

QT_BEGIN_NAMESPACE
class QQmlComponent;
class QQmlContext;
class QQuickItem;
QT_END_NAMESPACE

namespace oxide {

namespace qt {
class WebContextMenuClient;
}

namespace qquick {

class LegacyWebContextMenu : public qt::WebContextMenu {
 public:
  LegacyWebContextMenu(QQuickItem* parent,
                       QQmlComponent* component,
                       const qt::WebContextMenuParams& params,
                       qt::WebContextMenuClient* client);
  ~LegacyWebContextMenu() override;

 private:
  // qt::WebContextMenu implementation
  void Show() override;
  void Hide() override;

  qt::WebContextMenuParams params_;
  qt::WebContextMenuClient* client_;

  QPointer<QQuickItem> parent_;
  QPointer<QQmlComponent> component_;
  std::unique_ptr<QQuickItem> item_;
  std::unique_ptr<QQmlContext> context_;
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QT_QUICK_LEGACY_WEB_CONTEXT_MENU_H_
