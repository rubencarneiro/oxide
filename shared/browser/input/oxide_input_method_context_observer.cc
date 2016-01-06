// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#include "oxide_input_method_context_observer.h"

#include "oxide_input_method_context.h"

namespace oxide {

InputMethodContextObserver::InputMethodContextObserver()
    : im_context_(nullptr) {}

InputMethodContextObserver::InputMethodContextObserver(
    InputMethodContext* context)
    : im_context_(context) {
  if (context) {
    context->AddObserver(this);
  }
}

void InputMethodContextObserver::Observe(InputMethodContext* context) {
  if (context == im_context_) {
    return;
  }

  if (im_context_) {
    im_context_->RemoveObserver(this);
  }
  im_context_ = context;
  if (im_context_) {
    im_context_->AddObserver(this);
  }
}

void InputMethodContextObserver::OnInputMethodContextDestruction() {
  im_context_ = nullptr;
}

InputMethodContextObserver::~InputMethodContextObserver() {
  if (im_context_) {
    im_context_->RemoveObserver(this);
  }
}

void InputMethodContextObserver::InputPanelVisibilityChanged() {}

} // namespace oxide
