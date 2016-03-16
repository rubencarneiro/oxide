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

#ifndef _OXIDE_SHARED_BROWSER_INPUT_INPUT_METHOD_CONTEXT_H_
#define _OXIDE_SHARED_BROWSER_INPUT_INPUT_METHOD_CONTEXT_H_

#include "base/macros.h"
#include "base/observer_list.h"

namespace oxide {

class ImeBridge;
class ImeBridgeImpl;
class InputMethodContextObserver;

// This class is the interface from ImeBridgeImpl to the toolkit provided
// input method context, and provides a way for the toolkit layer to access
// the currently connected ImeBridge
class InputMethodContext {
 public:
  InputMethodContext();
  virtual ~InputMethodContext();

  virtual bool IsInputPanelVisible() const;

  virtual void TextInputStateChanged();
  virtual void SelectionBoundsChanged();
  virtual void SelectionChanged();
  virtual void CancelComposition();
  virtual void FocusedNodeChanged();

 protected:
  ImeBridge* ime_bridge() const;

  void NotifyInputPanelVisibilityChanged();

 private:
  friend class ImeBridgeImpl;
  friend class InputMethodContextObserver;

  void SetImeBridge(ImeBridgeImpl* bridge);

  void AddObserver(InputMethodContextObserver* observer);
  void RemoveObserver(InputMethodContextObserver* observer);

  bool in_destruction_;

  ImeBridgeImpl* ime_bridge_;

  base::ObserverList<InputMethodContextObserver> observers_;

  DISALLOW_COPY_AND_ASSIGN(InputMethodContext);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_INPUT_INPUT_METHOD_CONTEXT_H_
