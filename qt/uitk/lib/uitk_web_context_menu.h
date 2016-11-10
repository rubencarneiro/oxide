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

#ifndef _OXIDE_QT_UITK_LIB_WEB_CONTEXT_MENU_H_
#define _OXIDE_QT_UITK_LIB_WEB_CONTEXT_MENU_H_

#include <memory>
#include <vector>

#include <QObject>
#include <QPointer>
#include <QtGlobal>

#include "qt/core/glue/menu_item.h"
#include "qt/core/glue/web_context_menu.h"

QT_BEGIN_NAMESPACE
class QQuickItem;
QT_END_NAMESPACE

namespace oxide {

namespace qt {
struct MenuItem;
class WebContextMenuClient;
struct WebContextMenuParams;
}

namespace uitk {

class WebContextMenu : public QObject,
                       public qt::WebContextMenu {
  Q_OBJECT
  Q_DISABLE_COPY(WebContextMenu)

 public:
  static std::unique_ptr<WebContextMenu> Create(
      QQuickItem* parent,
      const qt::WebContextMenuParams& params,
      const std::vector<qt::MenuItem>& items,
      qt::WebContextMenuClient* client,
      bool mobile);
  ~WebContextMenu() override;

 private:
  WebContextMenu(QQuickItem* parent,
                 qt::WebContextMenuClient* client);
  bool Init(const std::vector<qt::MenuItem>& items,
            const qt::WebContextMenuParams& params,
            bool mobile);

  // qt::WebContextMenu implementation
  void Show() override;
  void Hide() override;

 private Q_SLOTS:
  void OnVisibleChanged();
  void OnCancelled();
  void OnActionTriggered(const QVariant&);

 private:
  QPointer<QQuickItem> parent_;
  qt::WebContextMenuClient* client_;

  std::vector<std::unique_ptr<QObject>> actions_;

  std::unique_ptr<QQuickItem> item_;
};

} // namespace uitk
} // namespace oxide

#endif // _OXIDE_QT_UITK_LIB_WEB_CONTEXT_MENU_H_
