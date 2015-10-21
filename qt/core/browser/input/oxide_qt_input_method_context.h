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

#ifndef _OXIDE_QT_CORE_BROWSER_INPUT_INPUT_METHOD_CONTEXT_H_
#define _OXIDE_QT_CORE_BROWSER_INPUT_INPUT_METHOD_CONTEXT_H_

#include <QObject>
#include <QtGlobal>
#include <QVariant>

#include "base/macros.h"

#include "shared/browser/input/oxide_input_method_context.h"

QT_BEGIN_NAMESPACE
class QFocusEvent;
class QInputMethodEvent;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class InputMethodContextClient;

class InputMethodContext : public QObject,
                           public oxide::InputMethodContext {
  Q_OBJECT

 public:
  InputMethodContext(InputMethodContextClient* client);
  ~InputMethodContext() override;

  QVariant Query(Qt::InputMethodQuery query) const;

  // Null out |client_| to prevent calls back in to it during its destructor
  void DetachClient();

  void FocusChanged(QFocusEvent* event);
  void HandleEvent(QInputMethodEvent* event);

 private Q_SLOTS:
  void OnInputPanelVisibilityChanged();

 private:
  bool ShouldShowInputPanel() const;
  bool ShouldHideInputPanel() const;

  void SetInputPanelVisibility(bool visible);

  // oxide::InputMethodContext implementation
  bool IsInputPanelVisible() const override;
  void TextInputStateChanged() override;
  void SelectionBoundsChanged() override;
  void SelectionChanged() override;
  void CancelComposition() override;
  void FocusedNodeChanged() override;

  InputMethodContextClient* client_; // Owns us

  bool has_input_method_state_;

  DISALLOW_COPY_AND_ASSIGN(InputMethodContext);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_INPUT_INPUT_METHOD_CONTEXT_H_
