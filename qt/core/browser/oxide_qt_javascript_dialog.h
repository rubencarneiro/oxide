// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include "shared/browser/oxide_javascript_dialog.h"

namespace oxide {
namespace qt {

class JavaScriptDialogDelegate;

class JavaScriptDialog FINAL : public oxide::JavaScriptDialog {
 public:
  JavaScriptDialog(JavaScriptDialogDelegate* delegate,
                   bool* did_suppress_message);

  void Run() FINAL;
  void Handle(bool accept, const base::string16* prompt_override) FINAL;

  QUrl originUrl() const;
  QString acceptLang() const;
  QString messageText() const;
  QString defaultPromptText() const;

 private:
  scoped_ptr<JavaScriptDialogDelegate> delegate_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(JavaScriptDialog);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_JAVASCRIPT_DIALOG_H_
