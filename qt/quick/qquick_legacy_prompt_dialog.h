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

#ifndef _OXIDE_QT_QUICK_LEGACY_PROMPT_DIALOG_H_
#define _OXIDE_QT_QUICK_LEGACY_PROMPT_DIALOG_H_

#include <QString>

#include "qt/quick/qquick_legacy_javascript_dialog.h"

namespace oxide {
namespace qquick {

class LegacyPromptDialog : public LegacyJavaScriptDialog {
 public:
  LegacyPromptDialog(QQuickItem* parent,
                     QQmlComponent* component,
                     const QString& message_text,
                     const QString& default_prompt_text,
                     qt::JavaScriptDialogClient* client);

 private:
  // qt::JavaScriptDialog implementation
  void Show() override;
  QString GetCurrentPromptText() override;

  QString message_text_;
  QString default_prompt_text_;
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QT_QUICK_LEGACY_PROMPT_DIALOG_H_
