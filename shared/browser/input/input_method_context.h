// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015-2016 Canonical Ltd.

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
#include "ui/base/ime/text_input_type.h"

#include "shared/common/oxide_shared_export.h"

namespace gfx {
class Range;
class Rect;
class SelectionBound;
}

namespace oxide {

class InputMethodContextClient;

class OXIDE_SHARED_EXPORT InputMethodContext {
 public:
  virtual ~InputMethodContext() = default;

  virtual bool IsInputPanelVisible() const = 0;

  virtual void TextInputStateChanged(ui::TextInputType type,
                                     int selection_start,
                                     int selection_end,
                                     bool show_ime_if_needed) = 0;

  virtual void SelectionBoundsChanged(const gfx::SelectionBound& anchor,
                                      const gfx::SelectionBound& focus,
                                      const gfx::Rect& caret_rect) = 0;

  virtual void TextSelectionChanged(size_t offset,
                                    const gfx::Range& range) = 0;

  virtual void FocusedNodeChanged(bool is_editable_node) = 0;

  virtual void CancelComposition() = 0;

  virtual void FocusIn() = 0;

  virtual void FocusOut() = 0;

 protected:
  InputMethodContext() = default;

  InputMethodContextClient* client() const { return client_; }

 private:
  friend class InputMethodContextClient;

  InputMethodContextClient* client_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(InputMethodContext);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_INPUT_INPUT_METHOD_CONTEXT_H_
