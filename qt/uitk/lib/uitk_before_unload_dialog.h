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

#ifndef _OXIDE_QT_UITK_LIB_BEFORE_UNLOAD_DIALOG_H_
#define _OXIDE_QT_UITK_LIB_BEFORE_UNLOAD_DIALOG_H_

#include <memory>

#include <QObject>
#include <QString>

#include "qt/core/glue/javascript_dialog.h"

QT_BEGIN_NAMESPACE
class QQuickItem;
QT_END_NAMESPACE

namespace oxide {

namespace qt {
class JavaScriptDialogClient;
}

namespace uitk {

class BeforeUnloadDialog : public QObject,
                           public qt::JavaScriptDialog {
  Q_OBJECT
  Q_DISABLE_COPY(BeforeUnloadDialog)

 public:
  ~BeforeUnloadDialog() override;

  static std::unique_ptr<BeforeUnloadDialog> Create(
      QQuickItem* parent,
      qt::JavaScriptDialogClient* client);

 private Q_SLOTS:
  void OnResponse(bool proceed);

 private:
  BeforeUnloadDialog(qt::JavaScriptDialogClient* client);
  bool Init(QQuickItem* parent);

  // qt::JavaScriptDialog implementation
  void Show() override;
  void Hide() override;
  QString GetCurrentPromptText() override;

  qt::JavaScriptDialogClient* client_;

  std::unique_ptr<QQuickItem> item_;
};

} // namespace uitk
} // namespace oxide

#endif // _OXIDE_QT_UITK_LIB_BEFORE_UNLOAD_DIALOG_H_
