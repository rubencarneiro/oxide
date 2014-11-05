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

#ifndef _OXIDE_QT_CORE_BROWSER_SCRIPT_MESSAGE_H_
#define _OXIDE_QT_CORE_BROWSER_SCRIPT_MESSAGE_H_

#include <string>

#include "base/basictypes.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "url/gurl.h"

namespace oxide {

class ScriptMessage;
class ScriptMessageImplBrowser;

namespace qt {

class ScriptMessageAdapter;
class WebFrame;

class ScriptMessage final {
 public:
  ~ScriptMessage();

  void Initialize(oxide::ScriptMessage* message);

  static ScriptMessage* FromAdapter(ScriptMessageAdapter* adapter);

  WebFrame* GetFrame() const;
  std::string GetMsgId() const;
  GURL GetContext() const;
  std::string GetArgs() const;

  void Reply(const std::string& args);
  void Error(const std::string& msg);

 private:
  friend class ScriptMessageAdapter;

  ScriptMessage(ScriptMessageAdapter* adapter);

  ScriptMessageAdapter* adapter_;
  scoped_refptr<oxide::ScriptMessageImplBrowser> impl_;

  DISALLOW_COPY_AND_ASSIGN(ScriptMessage);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_SCRIPT_MESSAGE_H_
