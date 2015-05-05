// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "url/gurl.h"

#include "qt/core/glue/oxide_qt_script_message_proxy.h"

namespace oxide {

class ScriptMessage;
class ScriptMessageImplBrowser;

namespace qt {

class ScriptMessage : public ScriptMessageProxy {
 public:
  ScriptMessage(oxide::ScriptMessage* message);
  ~ScriptMessage() override;

  static ScriptMessage* FromProxyHandle(ScriptMessageProxyHandle* handle);

 private:
  // ScriptMessageProxy implementation
  WebFrameProxyHandle* frame() const override;
  QString msgId() const override;
  QUrl context() const override;
  QVariant args() const override;
  void reply(const QVariant& args) override;
  void error(const QString& msg) override;

  scoped_refptr<oxide::ScriptMessageImplBrowser> impl_;
  mutable QVariant args_;

  DISALLOW_COPY_AND_ASSIGN(ScriptMessage);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_SCRIPT_MESSAGE_H_
