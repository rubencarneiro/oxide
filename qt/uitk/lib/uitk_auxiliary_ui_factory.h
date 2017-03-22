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

#ifndef _OXIDE_QT_UITK_LIB_AUXILIARY_UI_FACTORY_H_
#define _OXIDE_QT_UITK_LIB_AUXILIARY_UI_FACTORY_H_

#include <QtGlobal>

#include "qt/core/glue/auxiliary_ui_factory.h"

class OxideUbuntuWebContextMenu;

QT_BEGIN_NAMESPACE
class QQuickItem;
QT_END_NAMESPACE

namespace oxide {

namespace qt {
struct WebContextMenuParams;
}

namespace uitk {

class AuxiliaryUIFactory : public qt::AuxiliaryUIFactory {
 public:
  class Delegate {
   public:
    virtual ~Delegate();

    virtual void ContextMenuOpening(const qt::WebContextMenuParams& params,
                                    OxideUbuntuWebContextMenu* menu) = 0;
  };

  AuxiliaryUIFactory(QQuickItem* item,
                     Delegate* delegate = nullptr);
  ~AuxiliaryUIFactory() override;

 private:
  // qt::AuxiliaryUIFactory implementation
  std::unique_ptr<qt::WebContextMenu> CreateWebContextMenu(
      const qt::WebContextMenuParams& params,
      const std::vector<qt::MenuItem>& items,
      qt::WebContextMenuClient* client) override;
  std::unique_ptr<qt::TouchEditingMenu> CreateTouchEditingMenu(
      qt::EditCapabilityFlags edit_flags,
      qt::TouchEditingMenuClient* client) override;
  std::unique_ptr<qt::JavaScriptDialog> CreateJavaScriptDialog(
      const QUrl& origin_url,
      qt::JavaScriptDialogType type,
      const QString& message_text,
      const QString& default_prompt_text,
      qt::JavaScriptDialogClient* client) override;
  std::unique_ptr<qt::JavaScriptDialog> CreateBeforeUnloadDialog(
      const QUrl& origin_url,
      qt::JavaScriptDialogClient* client) override;

  QQuickItem* item_;

  Delegate* delegate_;

  Q_DISABLE_COPY(AuxiliaryUIFactory)
};

} // namespace uitk
} // namespace oxide

#endif // _OXIDE_QT_UITK_LIB_AUXILIARY_UI_FACTORY_H_
