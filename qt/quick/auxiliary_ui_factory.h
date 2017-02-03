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

#ifndef _OXIDE_QT_QUICK_AUXILIARY_UI_FACTORY_H_
#define _OXIDE_QT_QUICK_AUXILIARY_UI_FACTORY_H_

#include <memory>
#include <vector>

#include <QtGlobal>

#include "qt/core/glue/menu_item.h"
#include "qt/quick/api/oxideqquickglobal.h"

QT_BEGIN_NAMESPACE
class QQuickItem;
class QString;
class QUrl;
QT_END_NAMESPACE

namespace oxide {

namespace qt {
class JavaScriptDialog;
class JavaScriptDialogClient;
enum class JavaScriptDialogType;
class WebContextMenu;
class WebContextMenuClient;
struct WebContextMenuParams;
}

namespace qquick {

class OXIDE_QTQUICK_EXPORT AuxiliaryUIFactory {
 public:
  virtual ~AuxiliaryUIFactory();

  virtual std::unique_ptr<qt::WebContextMenu> CreateWebContextMenu(
      const qt::WebContextMenuParams& params,
      const std::vector<qt::MenuItem>& items,
      qt::WebContextMenuClient* client) = 0;

  virtual std::unique_ptr<qt::JavaScriptDialog> CreateJavaScriptDialog(
      const QUrl& origin_url,
      qt::JavaScriptDialogType type,
      const QString& message_text,
      const QString& default_prompt_text,
      qt::JavaScriptDialogClient* client) = 0;
  virtual std::unique_ptr<qt::JavaScriptDialog> CreateBeforeUnloadDialog(
      const QUrl& origin_url,
      qt::JavaScriptDialogClient* client) = 0;

 protected:
  AuxiliaryUIFactory(QQuickItem* item);

  QQuickItem* item() const { return item_; }

 private:
  QQuickItem* item_;
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QT_QUICK_AUXILIARY_UI_FACTORY_H_
