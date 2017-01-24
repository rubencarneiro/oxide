// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2016 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_BROWSER_JAVASCRIPT_DIALOG_HOST_H_
#define _OXIDE_QT_CORE_BROWSER_JAVASCRIPT_DIALOG_HOST_H_

#include <memory>

#include "base/macros.h"

#include "qt/core/glue/javascript_dialog_client.h"
#include "shared/browser/javascript_dialogs/javascript_dialog.h"

namespace oxide {

class JavaScriptDialogClient;

namespace qt {

class JavaScriptDialog;

class JavaScriptDialogHost : public oxide::JavaScriptDialog,
                             public JavaScriptDialogClient {
 public:
  JavaScriptDialogHost(oxide::JavaScriptDialogClient* client);
  ~JavaScriptDialogHost() override;

  void Init(std::unique_ptr<qt::JavaScriptDialog> dialog);

 private:
  // oxide::JavaScriptDialog implementation
  void Show() override;
  void Hide() override;
  base::string16 GetCurrentPromptText() override;

  // JavaScriptDialogClient implementation
  void close(bool accept, const QString& user_input) override;

  oxide::JavaScriptDialogClient* client_;

  std::unique_ptr<qt::JavaScriptDialog> dialog_;

  DISALLOW_COPY_AND_ASSIGN(JavaScriptDialogHost);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_JAVASCRIPT_DIALOG_HOST_H_
