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

#ifndef _OXIDE_UITK_LIB_WEB_POPUP_MENU_H_
#define _OXIDE_UITK_LIB_WEB_POPUP_MENU_H_

#include "qt/core/glue/menu_item.h"
#include "qt/core/glue/web_popup_menu.h"

#include <memory>
#include <vector>

#include <QObject>
#include <QRect>
#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QQuickItem;
QT_END_NAMESPACE

namespace oxide {

namespace qt {
class WebPopupMenuClient;
}

namespace uitk {

class WebPopupMenu : public QObject,
                     public qt::WebPopupMenu {
  Q_OBJECT

  Q_DISABLE_COPY(WebPopupMenu)

 public:
  static std::unique_ptr<WebPopupMenu> Create(
      QQuickItem* parent,
      const std::vector<qt::MenuItem>& items,
      const QRect& bounds,
      qt::WebPopupMenuClient* client);

  ~WebPopupMenu() override;

 private Q_SLOTS:
  void OnVisibleChanged();
  void OnItemSelected(int index);

 private:
  WebPopupMenu(const std::vector<qt::MenuItem>& items,
               const QRect& bounds,
               qt::WebPopupMenuClient* client);
  bool Init(QQuickItem* parent);

  // qt::WebPopupMenu implementation
  void Show() override;
  void Hide() override;

  class Model;
  std::unique_ptr<Model> model_;

  bool allow_multiple_selection_;

  QRect bounds_;

  qt::WebPopupMenuClient* client_;

  std::unique_ptr<QQuickItem> item_;
};

} // namespace uitk
} // namespace oxide

#endif // _OXIDE_UITK_LIB_WEB_POPUP_MENU_H_
