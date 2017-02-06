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

#ifndef _OXIDE_SHARED_BROWSER_JAVASCRIPT_DIALOGS_JAVASCRIPT_DIALOG_FACTORY_H_
#define _OXIDE_SHARED_BROWSER_JAVASCRIPT_DIALOGS_JAVASCRIPT_DIALOG_FACTORY_H_

#include <memory>

#include "content/public/common/javascript_message_type.h"

class GURL;

namespace oxide {

class JavaScriptDialog;
class JavaScriptDialogClient;

class JavaScriptDialogFactory {
 public:
  virtual ~JavaScriptDialogFactory() {}

  virtual std::unique_ptr<JavaScriptDialog> CreateBeforeUnloadDialog(
      JavaScriptDialogClient* client,
      const GURL& origin_url) = 0;

  virtual std::unique_ptr<JavaScriptDialog> CreateJavaScriptDialog(
      JavaScriptDialogClient* client,
      const GURL& origin_url,
      content::JavaScriptMessageType type,
      const base::string16& message_text,
      const base::string16& default_prompt_text) = 0;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_JAVASCRIPT_DIALOGS_JAVASCRIPT_DIALOG_FACTORY_H_
