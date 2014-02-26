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

#ifndef _OXIDE_QT_CORE_API_SCRIPT_MESSAGE_P_H_
#define _OXIDE_QT_CORE_API_SCRIPT_MESSAGE_P_H_

#include <QtGlobal>
#include <QVariant>

#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"

namespace oxide {
class ScriptMessage;
class ScriptMessageImplBrowser;
}

class OxideQScriptMessage;

class OxideQScriptMessagePrivate Q_DECL_FINAL {
 public:
  OxideQScriptMessagePrivate();

  oxide::ScriptMessageImplBrowser* incoming() const {
    return incoming_;
  }

  QVariant args() const { return args_; }

  void Initialize(oxide::ScriptMessage* message);
  void Consume(scoped_ptr<oxide::ScriptMessage> message);
  void Invalidate();

  static OxideQScriptMessagePrivate* get(OxideQScriptMessage* q);

  base::WeakPtr<OxideQScriptMessagePrivate> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

 private:
  oxide::ScriptMessageImplBrowser* incoming_;
  scoped_ptr<oxide::ScriptMessage> owned_incoming_;
  QVariant args_;
  base::WeakPtrFactory<OxideQScriptMessagePrivate> weak_factory_;
};

#endif // _OXIDE_QT_CORE_API_SCRIPT_MESSAGE_P_H_
