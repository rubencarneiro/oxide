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

#ifndef _OXIDE_QT_QUICK_LEGACY_AUXILIARY_UI_FACTORY_H_
#define _OXIDE_QT_QUICK_LEGACY_AUXILIARY_UI_FACTORY_H_

#include <QPointer>
#include <QtGlobal>

#include "qt/core/glue/auxiliary_ui_factory.h"

QT_BEGIN_NAMESPACE
class QQmlComponent;
class QQuickItem;
QT_END_NAMESPACE

namespace oxide {
namespace qquick {

class LegacyAuxiliaryUIFactory : public qt::AuxiliaryUIFactory {
  Q_DISABLE_COPY(LegacyAuxiliaryUIFactory)

 public:
  LegacyAuxiliaryUIFactory(QQuickItem* item);
  ~LegacyAuxiliaryUIFactory() override;

  QQmlComponent* context_menu() const { return context_menu_; }
  void set_context_menu(QQmlComponent* c) { context_menu_ = c; }

  QQmlComponent* alert_dialog() const { return alert_dialog_; }
  void set_alert_dialog(QQmlComponent* c) { alert_dialog_ = c; }

  QQmlComponent* confirm_dialog() const { return confirm_dialog_; }
  void set_confirm_dialog(QQmlComponent* c) { confirm_dialog_ = c; }

  QQmlComponent* prompt_dialog() const { return prompt_dialog_; }
  void set_prompt_dialog(QQmlComponent* c) { prompt_dialog_ = c; }

  QQmlComponent* before_unload_dialog() const { return before_unload_dialog_; }
  void set_before_unload_dialog(QQmlComponent* c) { before_unload_dialog_ = c; }

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

  QPointer<QQmlComponent> context_menu_;
  QPointer<QQmlComponent> alert_dialog_;
  QPointer<QQmlComponent> confirm_dialog_;
  QPointer<QQmlComponent> prompt_dialog_;
  QPointer<QQmlComponent> before_unload_dialog_;
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QT_QUICK_LEGACY_AUXILIARY_UI_FACTORY_H_
