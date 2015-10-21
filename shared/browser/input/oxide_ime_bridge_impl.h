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

#ifndef _OXIDE_SHARED_BROWSER_INPUT_IME_BRIDGE_IMPL_H_
#define _OXIDE_SHARED_BROWSER_INPUT_IME_BRIDGE_IMPL_H_

#include "base/macros.h"
#include "ui/base/ime/text_input_type.h"

#include "shared/browser/input/oxide_ime_bridge.h"

namespace gfx {
class Rect;
}

namespace oxide {

class InputMethodContext;
class RenderWidgetHostView;

class ImeBridgeImpl : public ImeBridge {
 public:
  ImeBridgeImpl(RenderWidgetHostView* rwhv);
  ~ImeBridgeImpl() override;

  InputMethodContext* context() const { return context_; }
  void SetContext(InputMethodContext* context);

  void TextInputStateChanged(ui::TextInputType type,
                             bool show_ime_if_needed);
  void SelectionBoundsChanged(const gfx::Rect& caret_rect,
                              size_t selection_cursor_position,
                              size_t selection_anchor_position);
  void FocusedNodeChanged(bool is_editable_node);

 private:
  base::string16 GetSelectionText() const override;
  base::string16 GetSelectedText() const override;
  void CommitText(const base::string16& text,
                  const gfx::Range& replacement_range) override;
  void SetComposingText(
      const base::string16& text,
      const std::vector<blink::WebCompositionUnderline>& underlines,
      const gfx::Range& selection_range) override;

  RenderWidgetHostView* rwhv_; // Owns us

  InputMethodContext* context_;

  DISALLOW_COPY_AND_ASSIGN(ImeBridgeImpl);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_INPUT_IME_BRIDGE_IMPL_H_
