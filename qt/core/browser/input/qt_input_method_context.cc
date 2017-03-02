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

#include "qt_input_method_context.h"

#include <algorithm>
#include <vector>

#include <QGuiApplication>
#include <QInputMethod>
#include <QInputMethodEvent>
#include <QString>
#include <QTextCharFormat>
#include <QWindow>

#include "base/numerics/safe_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/WebKit/public/platform/WebColor.h"
#include "third_party/WebKit/public/web/WebCompositionUnderline.h"

#include "qt/core/browser/oxide_qt_dpi_utils.h"
#include "qt/core/browser/oxide_qt_type_conversions.h"
#include "shared/browser/input/input_method_context_client.h"

#include "input_method_context_owner_client.h"

namespace oxide {
namespace qt {

namespace {

Qt::InputMethodHints InputTypeToImHints(ui::TextInputType type) {
  switch (type) {
    case ui::TEXT_INPUT_TYPE_TEXT:
    case ui::TEXT_INPUT_TYPE_TEXT_AREA:
    case ui::TEXT_INPUT_TYPE_CONTENT_EDITABLE:
      return Qt::ImhPreferLowercase;
    case ui::TEXT_INPUT_TYPE_PASSWORD:
      return Qt::ImhHiddenText | Qt::ImhSensitiveData |
          Qt::ImhNoAutoUppercase | Qt::ImhPreferLowercase |
          Qt::ImhNoPredictiveText;
    case ui::TEXT_INPUT_TYPE_SEARCH:
      return Qt::ImhNoAutoUppercase | Qt::ImhPreferLowercase;
    case ui::TEXT_INPUT_TYPE_EMAIL:
      return Qt::ImhEmailCharactersOnly;
    case ui::TEXT_INPUT_TYPE_NUMBER:
      return Qt::ImhFormattedNumbersOnly;
    case ui::TEXT_INPUT_TYPE_TELEPHONE:
      return Qt::ImhDialableCharactersOnly;
    case ui::TEXT_INPUT_TYPE_URL:
      return Qt::ImhUrlCharactersOnly;
    case ui::TEXT_INPUT_TYPE_DATE:
    case ui::TEXT_INPUT_TYPE_MONTH:
    case ui::TEXT_INPUT_TYPE_WEEK:
      return Qt::ImhDate;
    case ui::TEXT_INPUT_TYPE_DATE_TIME:
    case ui::TEXT_INPUT_TYPE_DATE_TIME_LOCAL:
    case ui::TEXT_INPUT_TYPE_DATE_TIME_FIELD:
      return Qt::ImhDate | Qt::ImhTime;
    case ui::TEXT_INPUT_TYPE_TIME:
      return Qt::ImhTime;
    default:
      return Qt::ImhNone;
  }
}

}

void InputMethodContext::OnInputPanelVisibilityChanged() {
  if (!client()) {
    return;
  }

  client()->InputPanelVisibilityChanged();
}

void InputMethodContext::UpdateInputMethodAcceptedState() {
  bool accepted =
      text_input_type_ != ui::TEXT_INPUT_TYPE_NONE || focused_node_is_editable_;
  owner_client_->SetInputMethodAccepted(accepted);
  NotifyPlatformOfUpdates(Qt::ImEnabled);
}

bool InputMethodContext::ShouldShowInputPanel() const {
  if (text_input_type_ != ui::TEXT_INPUT_TYPE_NONE &&
      show_ime_if_needed_ &&
      focused_node_is_editable_) {
    return true;
  }

  return false;
}

bool InputMethodContext::ShouldHideInputPanel() const {
  if (text_input_type_ == ui::TEXT_INPUT_TYPE_NONE &&
      !focused_node_is_editable_) {
    return true;
  }

  return false;
}

void InputMethodContext::SetInputPanelVisibility(bool visible) {
  if (!visible) {
    has_input_method_state_ = false;
  }

  // Do not check whether the input method is currently visible here, to avoid
  // a possible race condition: if hide() and show() are called very quickly 
  // in a row, when show() is called the hide() request might not have
  // completed yet, and isVisible() could return true.
  // See https://launchpad.net/bugs/1377755
  QInputMethod* im = QGuiApplication::inputMethod();
  if (im) {
    im->setVisible(visible);
  }
}

void InputMethodContext::NotifyPlatformOfUpdates(
    Qt::InputMethodQueries queries) {
  // Don't notify when switching between fields. Technically we should only
  // defer ImHints, however, the maliit plugin seems to update everything
  // regardless of the flags we pass to QInputMethod:update()
  // See https://launchpad.net/bugs/1381083
  if (text_input_type_ == ui::TEXT_INPUT_TYPE_NONE &&
      focused_node_is_editable_) {
    deferred_update_queries_ |= queries;
    return;
  }

  queries |= deferred_update_queries_;
  deferred_update_queries_ = 0;

  if (queries == 0) {
    return;
  }

  QInputMethod* im = QGuiApplication::inputMethod();
  if (im) {
    im->update(queries);
  }
}

bool InputMethodContext::IsInputPanelVisible() const {
  QInputMethod* im = QGuiApplication::inputMethod();
  if (!im) {
    return false;
  }

  return im->isVisible();
}

void InputMethodContext::TextInputStateChanged(ui::TextInputType type,
                                               int selection_start,
                                               int selection_end,
                                               bool show_ime_if_needed) {
  if (type == text_input_type_ &&
      selection_start == selection_start_ &&
      selection_end == selection_end_ &&
      show_ime_if_needed == show_ime_if_needed_) {
    return;
  }

  text_input_type_ = type;
  selection_start_ = selection_start;
  selection_end_ = selection_end;
  show_ime_if_needed_ = show_ime_if_needed;

  UpdateInputMethodAcceptedState();

  if (!has_focus_) {
    return;
  }

  NotifyPlatformOfUpdates(static_cast<Qt::InputMethodQueries>(
      Qt::ImHints | Qt::ImCursorPosition | Qt::ImAnchorPosition
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
      | Qt::ImTextBeforeCursor | Qt::ImTextAfterCursor
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
      | Qt::ImAbsolutePosition
#endif
  ));

  if (ShouldShowInputPanel()) {
    SetInputPanelVisibility(true);
  } else if (ShouldHideInputPanel()) {
    SetInputPanelVisibility(false);
  }
}

void InputMethodContext::SelectionBoundsChanged(
    const gfx::SelectionBound& anchor,
    const gfx::SelectionBound& focus,
    const gfx::Rect& caret_rect) {
  if (caret_rect == caret_rect_) {
    return;
  }

  caret_rect_ = caret_rect;

  if (!has_focus_) {
    return;
  }

  NotifyPlatformOfUpdates(Qt::ImCursorRectangle);
}

void InputMethodContext::TextSelectionChanged(size_t offset,
                                              const gfx::Range& range) {
  selection_offset_ = offset;
  selection_range_ = range;

  if (!has_focus_) {
    return;
  }

  NotifyPlatformOfUpdates(static_cast<Qt::InputMethodQueries>(
      Qt::ImSurroundingText | Qt::ImCurrentSelection
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
        | Qt::ImTextBeforeCursor | Qt::ImTextAfterCursor
#endif
  ));
}

void InputMethodContext::FocusedNodeChanged(bool is_editable_node) {
  if (is_editable_node != focused_node_is_editable_) {
    // Work around for https://launchpad.net/bugs/1323743
    // Only run this code when is_editable_node changes otherwise it
    // reintroduces https://launchpad.net/bugs/1381083
    if (QGuiApplication::focusWindow() &&
        QGuiApplication::focusWindow()->focusObject()) {
      QGuiApplication::focusWindow()->focusObjectChanged(
          QGuiApplication::focusWindow()->focusObject());
    }
  }

  focused_node_is_editable_ = is_editable_node;

  UpdateInputMethodAcceptedState();

  if (!has_focus_) {
    return;
  }

  if (ShouldHideInputPanel()) {
    SetInputPanelVisibility(false);
  } else if (!has_input_method_state_ && ShouldShowInputPanel()) {
    // See https://launchpad.net/bugs/1400372
    SetInputPanelVisibility(true);
  } else if (has_input_method_state_ && focused_node_is_editable_) {
    CancelComposition();
  }

  // Flush deferred updates
  NotifyPlatformOfUpdates(0);
}

void InputMethodContext::CancelComposition() {
  if (!has_input_method_state_) {
    return;
  }

  has_input_method_state_ = false;

  if (!has_focus_) {
    return;
  }

  QInputMethod* im = QGuiApplication::inputMethod();
  if (!im) {
    return;
  }

  im->reset();
}

void InputMethodContext::FocusIn() {
  has_focus_ = true;

  if (!ShouldShowInputPanel()) {
    return;
  }

  SetInputPanelVisibility(true);
}

void InputMethodContext::FocusOut() {
  has_focus_ = false;
}

InputMethodContext::InputMethodContext(
    InputMethodContextOwnerClient* owner_client)
    : owner_client_(owner_client) {
  QInputMethod* im = QGuiApplication::inputMethod();
  if (im) {
    connect(im, &QInputMethod::visibleChanged,
            this, &InputMethodContext::OnInputPanelVisibilityChanged);
  }
}

InputMethodContext::~InputMethodContext() = default;

QVariant InputMethodContext::Query(Qt::InputMethodQuery query) const {
  switch (query) {
    case Qt::ImHints:
      return QVariant(InputTypeToImHints(text_input_type_));
    case Qt::ImCursorRectangle:
      return ToQt(
          DpiUtils::ConvertChromiumPixelsToQt(caret_rect_,
                                              owner_client_->GetScreen()));
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    case Qt::ImAbsolutePosition:
#endif
    case Qt::ImCursorPosition:
      return selection_end_;
    case Qt::ImSurroundingText:
      if (!client()) {
        return QString();
      }
      return QString::fromStdString(
          base::UTF16ToUTF8(client()->GetSelectionText()));
    case Qt::ImCurrentSelection: {
      if (!client()) {
        return QString();
      }
      base::string16 text;
      if (!client()->GetSelectedText(&text)) {
        return QString();
      }
      return QString::fromStdString(base::UTF16ToUTF8(text));
    }
    case Qt::ImAnchorPosition:
      return selection_start_;
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
    case Qt::ImTextBeforeCursor: {
      if (!client()) {
        return QString();
      }
      if (selection_end_ < 0) {
        return QString();
      }
      std::string text = base::UTF16ToUTF8(client()->GetSelectionText());
      return QString::fromStdString(text.substr(0, selection_end_));
    }
    case Qt::ImTextAfterCursor: {
      if (!client()) {
        return QString();
      }
      if (selection_end_ < 0) {
        return QString();
      }
      std::string text = base::UTF16ToUTF8(client()->GetSelectionText());
      if (base::checked_cast<size_t>(selection_end_) > text.length()) {
        return QString();
      }
      return QString::fromStdString(
          text.substr(selection_end_, std::string::npos));
    }
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    case Qt::ImAnchorRectangle:
      // TODO:
      return QRect();
#endif
    default:
      break;
  }

  return QVariant();
}

void InputMethodContext::HandleEvent(QInputMethodEvent* event) {
  QString commit_string = event->commitString();

  if (!commit_string.isEmpty()) {
    gfx::Range replacement_range = gfx::Range::InvalidRange();
    if (event->replacementLength() > 0) {
      replacement_range.set_start(event->replacementStart());
      replacement_range.set_end(event->replacementStart() +
                                event->replacementLength());
    }
    if (client()) {
      client()->CommitText(base::UTF8ToUTF16(commit_string.toStdString()),
                           replacement_range);
    }
  }

  QString preedit_string = event->preeditString();

  std::vector<blink::WebCompositionUnderline> underlines;
  int cursor_position = -1;
  gfx::Range selection_range = gfx::Range::InvalidRange();

  for (const auto& attribute : event->attributes()) {
    switch (attribute.type) {
      case QInputMethodEvent::Cursor:
        if (attribute.length > 0) {
          cursor_position = attribute.start;
        }
        break;
      case QInputMethodEvent::Selection:
        selection_range.set_start(
            std::min(attribute.start, (attribute.start + attribute.length)));
        selection_range.set_end(
            std::max(attribute.start, (attribute.start + attribute.length)));
        break;
      case QInputMethodEvent::TextFormat: {
        QTextCharFormat format =
            attribute.value.value<QTextFormat>().toCharFormat();
        blink::WebColor color = format.underlineColor().rgba();
        int start =
            std::min(attribute.start, (attribute.start + attribute.length));
        int end =
            std::max(attribute.start, (attribute.start + attribute.length));
        blink::WebCompositionUnderline underline(
            start, end, color, false, SK_ColorTRANSPARENT);
        underlines.push_back(underline);
        break;
      }
      default:
        break;
    }
  }

  if (!selection_range.IsValid()) {
    selection_range =
        gfx::Range(cursor_position > 0 ?
                       cursor_position : preedit_string.length());
  }

  if (client()) {
    client()->SetComposingText(
        base::UTF8ToUTF16(preedit_string.toStdString()),
        underlines, selection_range);
  }

  has_input_method_state_ = !preedit_string.isEmpty();
}

} // namespace qt
} // namespace oxide
