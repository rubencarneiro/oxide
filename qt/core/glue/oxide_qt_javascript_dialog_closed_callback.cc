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

#include "oxide_qt_javascript_dialog_closed_callback.h"

#include "base/strings/utf_string_conversions.h"

#include "qt/core/glue/private/oxide_qt_javascript_dialog_closed_callback_p.h"

namespace oxide {
namespace qt {

JavaScriptDialogClosedCallback::JavaScriptDialogClosedCallback() {
}

JavaScriptDialogClosedCallback::~JavaScriptDialogClosedCallback() {
}

void JavaScriptDialogClosedCallback::run(bool success,
                                         const QString& user_input) const {
  priv->callback_.Run(success, base::UTF8ToUTF16(user_input.toStdString()));
  delete this;
}

} // namespace qt
} // namespace oxide
