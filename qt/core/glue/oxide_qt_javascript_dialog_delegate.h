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

#ifndef _OXIDE_QT_CORE_GLUE_JAVASCRIPT_DIALOG_DELEGATE_H_
#define _OXIDE_QT_CORE_GLUE_JAVASCRIPT_DIALOG_DELEGATE_H_

#include <QtGlobal>
#include <QString>
#include <QUrl>

namespace oxide {
namespace qt {

class JavaScriptDialog;

class Q_DECL_EXPORT JavaScriptDialogDelegate {
 public:
  enum Type {
    TypeAlert,
    TypeConfirm,
    TypePrompt
  };

  virtual ~JavaScriptDialogDelegate();

  virtual bool Show() = 0;
  void Close(bool accept, const QString& user_input = QString());
  virtual void Handle(bool accept, const QString& prompt_override);

 protected:
  JavaScriptDialogDelegate();

  friend class JavaScriptDialog;

  QUrl originUrl() const;
  QString acceptLang() const;
  QString messageText() const;
  QString defaultPromptText() const;

private:
  JavaScriptDialog* dialog_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_JAVASCRIPT_DIALOG_DELEGATE_H_
