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

#ifndef _OXIDE_QT_QUICK_API_SCRIPT_MESSAGE_P_P_H_
#define _OXIDE_QT_QUICK_API_SCRIPT_MESSAGE_P_P_H_

#include <QtGlobal>

#include "qt/core/glue/oxide_qt_script_message_adapter.h"

class OxideQQuickScriptMessage;

class OxideQQuickScriptMessagePrivate Q_DECL_FINAL :
    public oxide::qt::ScriptMessageAdapter {
 public:
  OxideQQuickScriptMessagePrivate(OxideQQuickScriptMessage* q);

  static OxideQQuickScriptMessagePrivate* get(OxideQQuickScriptMessage* q);
};

#endif // _OXIDE_QT_QUICK_API_SCRIPT_MESSAGE_P_P_H_
