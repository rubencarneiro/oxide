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

#ifndef _OXIDE_SHARED_BROWSER_INPUT_IME_BRIDGE_H_
#define _OXIDE_SHARED_BROWSER_INPUT_IME_BRIDGE_H_

#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "third_party/WebKit/public/web/WebCompositionUnderline.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/gfx/geometry/rect.h"

namespace gfx {
class Range;
}

namespace oxide {

// This class contains state for a particular RWHV. InputMethodContext is
// connected to one ImeBridge
class ImeBridge {
 public:
  ImeBridge();
  virtual ~ImeBridge();

  ui::TextInputType text_input_type() const { return text_input_type_; }

  bool show_ime_if_needed() const { return show_ime_if_needed_; }

  gfx::Rect caret_rect() const { return caret_rect_; }

  size_t selection_cursor_position() const {
    return selection_cursor_position_;
  }

  size_t selection_anchor_position() const {
    return selection_anchor_position_;
  }

  bool focused_node_is_editable() const {
    return focused_node_is_editable_;
  }

  virtual base::string16 GetSelectionText() const = 0;
  virtual base::string16 GetSelectedText() const = 0;

  virtual void CommitText(const base::string16& text,
                          const gfx::Range& replacement_range) = 0;
  virtual void SetComposingText(
      const base::string16& text,
      const std::vector<blink::WebCompositionUnderline>& underlines,
      const gfx::Range& selection_range) = 0;

 protected:
  ui::TextInputType text_input_type_;
  bool show_ime_if_needed_;
  gfx::Rect caret_rect_;
  size_t selection_cursor_position_;
  size_t selection_anchor_position_;
  bool focused_node_is_editable_;

 private:
  DISALLOW_COPY_AND_ASSIGN(ImeBridge);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_INPUT_IME_BRIDGE_H_
