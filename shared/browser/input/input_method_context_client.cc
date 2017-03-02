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

#include "input_method_context_client.h"

#include "base/logging.h"

#include "input_method_context.h"

namespace oxide {

InputMethodContextClient::InputMethodContextClient() = default;

void InputMethodContextClient::AttachToContext(InputMethodContext* context) {
  if (context_) {
    DCHECK_EQ(context_->client_, this);
    context_->client_ = nullptr;
  }

  context_ = context;

  if (context_) {
    DCHECK(!context_->client_);
    context_->client_ = this;
  }
}

void InputMethodContextClient::DetachFromContext() {
  AttachToContext(nullptr);
}

InputMethodContextClient::~InputMethodContextClient() = default;

} // namespace oxide
