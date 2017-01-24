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

#include "javascript_dialog_testing_utils.h"

#include "base/bind.h"

namespace oxide {

namespace {

void TestCallback(int* count_out,
                  bool* success_out,
                  base::string16* user_input_out,
                  bool success,
                  const base::string16& user_input) {
  ++(*count_out);
  if (success_out) {
    *success_out = success;
  }
  if (user_input_out) {
    *user_input_out = user_input;
  }
}

}

content::JavaScriptDialogManager::DialogClosedCallback
MakeJavaScriptDialogTestCallback(int* count_out,
                                 bool* success_out,
                                 base::string16* user_input_out) {
  return base::Bind(&TestCallback, count_out, success_out, user_input_out);
}

} // namespace oxide
