// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_GLUE_PRIVATE_JAVASCRIPT_DIALOG_CLOSED_CALLBACK_H_
#define _OXIDE_QT_CORE_GLUE_PRIVATE_JAVASCRIPT_DIALOG_CLOSED_CALLBACK_H_

#include "base/basictypes.h"

#include "content/public/browser/javascript_dialog_manager.h"

namespace oxide {
namespace qt {

class JavaScriptDialogClosedCallback;

class JavaScriptDialogClosedCallbackPrivate FINAL {
 public:
  static JavaScriptDialogClosedCallback* CreateCallbackWrapper(
      const content::JavaScriptDialogManager::DialogClosedCallback& callback);

 private:
  friend class JavaScriptDialogClosedCallback;
  JavaScriptDialogClosedCallbackPrivate(
      const content::JavaScriptDialogManager::DialogClosedCallback& callback);
  DISALLOW_COPY_AND_ASSIGN(JavaScriptDialogClosedCallbackPrivate);
  const content::JavaScriptDialogManager::DialogClosedCallback& callback_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_PRIVATE_JAVASCRIPT_DIALOG_CLOSED_CALLBACK_H_
