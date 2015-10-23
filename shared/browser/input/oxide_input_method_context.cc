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

#include "oxide_input_method_context.h"

#include "oxide_input_method_context_observer.h"

namespace oxide {

void InputMethodContext::NotifyInputPanelVisibilityChanged() {
  FOR_EACH_OBSERVER(InputMethodContextObserver,
                    observers_,
                    InputPanelVisibilityChanged());
}

void InputMethodContext::SetImeBridge(ImeBridge* bridge) {
  ime_bridge_ = bridge;

  CancelComposition();

  // XXX: This is a bit of a hammer
  TextInputStateChanged();
  SelectionBoundsChanged();
  SelectionChanged();
  FocusedNodeChanged();
}

void InputMethodContext::AddObserver(InputMethodContextObserver* observer) {
  observers_.AddObserver(observer);
}

void InputMethodContext::RemoveObserver(InputMethodContextObserver* observer) {
  observers_.RemoveObserver(observer);
}

InputMethodContext::InputMethodContext()
    : ime_bridge_(nullptr) {}

InputMethodContext::~InputMethodContext() {
  FOR_EACH_OBSERVER(InputMethodContextObserver,
                    observers_,
                    OnInputMethodContextDestruction());
}

bool InputMethodContext::IsInputPanelVisible() const {
  return false;
}

void InputMethodContext::TextInputStateChanged() {}
void InputMethodContext::SelectionBoundsChanged() {}
void InputMethodContext::SelectionChanged() {}
void InputMethodContext::CancelComposition() {}
void InputMethodContext::FocusedNodeChanged() {}

} // namespace oxide
