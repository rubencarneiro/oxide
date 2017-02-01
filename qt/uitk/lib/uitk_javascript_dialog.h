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

#ifndef _OXIDE_QT_UITK_LIB_JAVASCRIPT_DIALOG_H_
#define _OXIDE_QT_UITK_LIB_JAVASCRIPT_DIALOG_H_

#include <memory>

#include <QObject>
#include <QString>

#include "qt/core/glue/javascript_dialog.h"
#include "qt/core/glue/javascript_dialog_type.h"

QT_BEGIN_NAMESPACE
class QQuickItem;
QT_END_NAMESPACE

namespace oxide {

namespace qt {
class JavaScriptDialogClient;
}

namespace uitk {

class JavaScriptDialog : public QObject,
                         public qt::JavaScriptDialog {
  Q_OBJECT
  Q_DISABLE_COPY(JavaScriptDialog)

 public:
  ~JavaScriptDialog() override;

  static std::unique_ptr<JavaScriptDialog> Create(
      QQuickItem* parent,
      qt::JavaScriptDialogType type,
      const QString& message_text,
      const QString& default_prompt_text,
      qt::JavaScriptDialogClient* client);

 private Q_SLOTS:
  void OnResponse(bool success, const QString& user_input);

 private:
  JavaScriptDialog(qt::JavaScriptDialogClient* client);
  bool Init(QQuickItem* parent,
            qt::JavaScriptDialogType type,
            const QString& message_text,
            const QString& default_prompt_text);

  // qt::JavaScriptDialog implementation
  void Show() override;
  void Hide() override;
  QString GetCurrentPromptText() override;

  qt::JavaScriptDialogClient* client_;

  std::unique_ptr<QQuickItem> item_;
};

} // namespace uitk
} // namespace oxide

#endif // _OXIDE_QT_UITK_LIB_JAVASCRIPT_DIALOG_H_
