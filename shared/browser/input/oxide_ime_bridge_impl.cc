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

#include "oxide_ime_bridge_impl.h"

#include "content/browser/renderer_host/render_widget_host_impl.h" // nogncheck
#include "content/public/browser/native_web_keyboard_event.h"
#include "third_party/WebKit/public/platform/WebInputEvent.h"
#include "ui/events/keycodes/dom/dom_key.h"
#include "ui/events/keycodes/keyboard_codes.h"
#include "ui/gfx/range/range.h"

#include "shared/browser/oxide_render_widget_host_view.h"

#include "oxide_input_method_context.h"

namespace oxide {

namespace {

// Qt input methods don’t generate key events, but a lot of web pages out there
// rely on keydown and keyup events to e.g. perform search-as-you-type or
// enable/disable a submit button based on the contents of a text input field,
// so we send a fake pair of keydown/keyup events.
// This mimicks what is done in GtkIMContextWrapper::HandlePreeditChanged(…)
// and GtkIMContextWrapper::HandleCommit(…)
// (see content/browser/renderer_host/gtk_im_context_wrapper.cc).
void SendFakeCompositionKeyEvent(content::RenderWidgetHostImpl* host,
                                 blink::WebInputEvent::Type type) {
  content::NativeWebKeyboardEvent fake_event(type, 0, base::TimeTicks::Now());
  fake_event.windowsKeyCode = ui::VKEY_PROCESSKEY;
  fake_event.skip_in_browser = true;
  fake_event.domKey = ui::DomKey::Key::PROCESS;
  host->ForwardKeyboardEvent(fake_event);
}

}

base::string16 ImeBridgeImpl::GetSelectionText() const {
  return rwhv_->GetSelectionText();
}

base::string16 ImeBridgeImpl::GetSelectedText() const {
  return rwhv_->GetSelectedText();
}

void ImeBridgeImpl::CommitText(const base::string16& text,
                               const gfx::Range& replacement_range) {
  if (!rwhv_->GetRenderWidgetHost()) {
    return;
  }

  rwhv_->OnUserInput();

  content::RenderWidgetHostImpl* rwhi =
      content::RenderWidgetHostImpl::From(rwhv_->GetRenderWidgetHost());
  SendFakeCompositionKeyEvent(rwhi, blink::WebInputEvent::RawKeyDown);
  rwhi->ImeCommitText(text,
                      std::vector<blink::WebCompositionUnderline>(),
                      replacement_range,
                      false);
  SendFakeCompositionKeyEvent(rwhi, blink::WebInputEvent::KeyUp);
}

void ImeBridgeImpl::SetComposingText(
    const base::string16& text,
    const std::vector<blink::WebCompositionUnderline>& underlines,
    const gfx::Range& selection_range) {
  if (!rwhv_->GetRenderWidgetHost()) {
    return;
  }

  rwhv_->OnUserInput();

  content::RenderWidgetHostImpl* rwhi =
      content::RenderWidgetHostImpl::From(rwhv_->GetRenderWidgetHost());
  SendFakeCompositionKeyEvent(rwhi, blink::WebInputEvent::RawKeyDown);
  rwhi->ImeSetComposition(text,
                          underlines,
                          gfx::Range::InvalidRange(),
                          selection_range.start(),
                          selection_range.end());
  SendFakeCompositionKeyEvent(rwhi, blink::WebInputEvent::KeyUp);
}

ImeBridgeImpl::ImeBridgeImpl(RenderWidgetHostView* rwhv)
    : rwhv_(rwhv),
      context_(nullptr) {}

ImeBridgeImpl::~ImeBridgeImpl() {
  SetContext(nullptr);
}

void ImeBridgeImpl::SetContext(InputMethodContext* context) {
  if (context_) {
    DCHECK_EQ(context_->ime_bridge(), this);
    context_->SetImeBridge(nullptr);
  }
  context_ = context;
  if (context_) {
    DCHECK(!context_->ime_bridge());
    context_->SetImeBridge(this);
  }
}

void ImeBridgeImpl::TextInputStateChanged(ui::TextInputType type,
                                          bool show_ime_if_needed) {
  if (type == text_input_type_ &&
      show_ime_if_needed == show_ime_if_needed_) {
    return;
  }

  text_input_type_ = type;
  show_ime_if_needed_ = show_ime_if_needed;

  if (!context_) {
    return;
  }

  context_->TextInputStateChanged();
}

void ImeBridgeImpl::SelectionBoundsChanged(const gfx::Rect& caret_rect,
                                           size_t selection_cursor_position,
                                           size_t selection_anchor_position) {
  if (caret_rect == caret_rect_ &&
      selection_cursor_position == selection_cursor_position_ &&
      selection_anchor_position == selection_anchor_position_) {
    return;
  }

  caret_rect_ = caret_rect;
  selection_cursor_position_ = selection_cursor_position;
  selection_anchor_position_ = selection_anchor_position;

  if (!context_) {
    return;
  }

  context_->SelectionBoundsChanged();
}

void ImeBridgeImpl::FocusedNodeChanged(bool is_editable_node) {
  if (is_editable_node == focused_node_is_editable_) {
    return;
  }

  focused_node_is_editable_ = is_editable_node;

  if (!context_) {
    return;
  }

  context_->FocusedNodeChanged();
}

} // namespace oxide
