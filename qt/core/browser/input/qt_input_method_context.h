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

#ifndef _OXIDE_QT_CORE_BROWSER_INPUT_INPUT_METHOD_CONTEXT_H_
#define _OXIDE_QT_CORE_BROWSER_INPUT_INPUT_METHOD_CONTEXT_H_

#include <QObject>
#include <Qt>
#include <QVariant>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/range/range.h"

#include "shared/browser/input/input_method_context.h"

QT_BEGIN_NAMESPACE
class QInputMethodEvent;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class InputMethodContextOwnerClient;

class InputMethodContext : public QObject,
                           public oxide::InputMethodContext {
  Q_OBJECT

 public:
  InputMethodContext(InputMethodContextOwnerClient* owner_client);
  ~InputMethodContext() override;

  QVariant Query(Qt::InputMethodQuery query) const;

  void HandleEvent(QInputMethodEvent* event);

 private Q_SLOTS:
  void OnInputPanelVisibilityChanged();

 private:
  void UpdateInputMethodAcceptedState();

  bool ShouldShowInputPanel() const;
  bool ShouldHideInputPanel() const;

  void SetInputPanelVisibility(bool visible);

  void NotifyPlatformOfUpdates(Qt::InputMethodQueries queries);

  // oxide::InputMethodContext implementation
  bool IsInputPanelVisible() const override;
  void TextInputStateChanged(ui::TextInputType type,
                             int selection_start,
                             int selection_end,
                             bool show_ime_if_needed) override;
  void SelectionBoundsChanged(const gfx::SelectionBound& anchor,
                              const gfx::SelectionBound& focus,
                              const gfx::Rect& caret_rect) override;
  void TextSelectionChanged(size_t offset,
                            const gfx::Range& range) override;
  void FocusedNodeChanged(bool is_editable_node) override;
  void CancelComposition() override;
  void FocusIn() override;
  void FocusOut() override;

  InputMethodContextOwnerClient* owner_client_;

  ui::TextInputType text_input_type_ = ui::TEXT_INPUT_TYPE_NONE;
  int selection_start_ = -1;
  int selection_end_ = -1;
  bool show_ime_if_needed_ = false;

  gfx::Rect caret_rect_;

  size_t selection_offset_ = 0;
  gfx::Range selection_range_;

  bool focused_node_is_editable_ = false;

  bool has_focus_ = false;

  bool has_input_method_state_ = false;

  Qt::InputMethodQueries deferred_update_queries_;

  DISALLOW_COPY_AND_ASSIGN(InputMethodContext);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_INPUT_INPUT_METHOD_CONTEXT_H_
