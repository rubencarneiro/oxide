// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_BROWSER_JAVASCRIPT_DIALOG_H_
#define _OXIDE_QT_CORE_BROWSER_JAVASCRIPT_DIALOG_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"

#include <QString>
#include <QUrl>

#include "qt/core/glue/oxide_qt_javascript_dialog_proxy_client.h"
#include "shared/browser/oxide_javascript_dialog.h"

namespace oxide {
namespace qt {

class JavaScriptDialogProxy;

class JavaScriptDialog : public oxide::JavaScriptDialog,
                         public JavaScriptDialogProxyClient {
 public:
  JavaScriptDialog();

  void SetProxy(JavaScriptDialogProxy* proxy);

 private:
  ~JavaScriptDialog() override;

  // oxide::JavaScriptDialog implementation
  bool Run() override;
  void Hide() override;
  void Handle(bool accept, const base::string16* prompt_override) override;

  // JavaScriptDialogProxyClient implementation
  void close(bool accept, const QString& user_input = QString()) override;
  QUrl originUrl() const override;
  QString messageText() const override;
  QString defaultPromptText() const override;

  scoped_ptr<JavaScriptDialogProxy> proxy_;

  DISALLOW_COPY_AND_ASSIGN(JavaScriptDialog);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_JAVASCRIPT_DIALOG_H_
