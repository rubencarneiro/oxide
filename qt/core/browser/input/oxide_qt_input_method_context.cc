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

#include "oxide_qt_input_method_context.h"

#include <string>
#include <vector>

#include <QFocusEvent>
#include <QGuiApplication>
#include <QInputMethod>
#include <QInputMethodEvent>
#include <QRect>
#include <QString>
#include <QTextCharFormat>
#include <QWindow>

#include "base/strings/utf_string_conversions.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/WebKit/public/platform/WebColor.h"
#include "third_party/WebKit/public/web/WebCompositionUnderline.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/range/range.h"

#include "shared/browser/input/oxide_ime_bridge.h"

#include "oxide_qt_input_method_context_client.h"

namespace oxide {
namespace qt {

namespace {

Qt::InputMethodHints QImHintsFromInputType(ui::TextInputType type) {
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
  NotifyInputPanelVisibilityChanged();
}

bool InputMethodContext::ShouldShowInputPanel() const {
  if (!ime_bridge()) {
    return false;
  }

  if (ime_bridge()->text_input_type() != ui::TEXT_INPUT_TYPE_NONE &&
      ime_bridge()->show_ime_if_needed() &&
      ime_bridge()->focused_node_is_editable()) {
    return true;
  }

  return false;
}

bool InputMethodContext::ShouldHideInputPanel() const {
  if (!ime_bridge()) {
    return true;
  }

  if (ime_bridge()->text_input_type() == ui::TEXT_INPUT_TYPE_NONE &&
      !ime_bridge()->focused_node_is_editable()) {
    return true;
  }

  return false;
}

void InputMethodContext::SetInputPanelVisibility(bool visible) {
  client_->SetInputMethodEnabled(visible);

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

bool InputMethodContext::IsInputPanelVisible() const {
  QInputMethod* im = QGuiApplication::inputMethod();
  if (!im) {
    return false;
  }

  return im->isVisible();
}

void InputMethodContext::TextInputStateChanged() {
  if (!client_) {
    return;
  }

  if (!client_->HasFocus()) {
    return;
  }

  // Don't notify if type is none. See https://launchpad.net/bugs/1381083
  if (ime_bridge() &&
      ime_bridge()->text_input_type() != ui::TEXT_INPUT_TYPE_NONE) {
    QGuiApplication::inputMethod()->update(
        static_cast<Qt::InputMethodQueries>(Qt::ImQueryInput | Qt::ImHints));
  }

  if (ShouldShowInputPanel()) {
    SetInputPanelVisibility(true);
  } else if (ShouldHideInputPanel()) {
    SetInputPanelVisibility(false);
  }
}

void InputMethodContext::SelectionBoundsChanged() {
  if (!client_) {
    return;
  }

  if (!client_->HasFocus()) {
    return;
  }

  QInputMethod* im = QGuiApplication::inputMethod();
  if (!im) {
    return;
  }

  im->update(static_cast<Qt::InputMethodQueries>(
      Qt::ImCursorRectangle
      | Qt::ImCursorPosition
      | Qt::ImAnchorPosition
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
      | Qt::ImTextBeforeCursor
      | Qt::ImTextAfterCursor
#endif
  ));
}

void InputMethodContext::SelectionChanged() {
  if (!client_) {
    return;
  }

  if (!client_->HasFocus()) {
    return;
  }

  QInputMethod* im = QGuiApplication::inputMethod();
  if (!im) {
    return;
  }

  im->update(static_cast<Qt::InputMethodQueries>(
      Qt::ImSurroundingText
      | Qt::ImCurrentSelection
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
      | Qt::ImTextBeforeCursor
      | Qt::ImTextAfterCursor
#endif
  ));
}

void InputMethodContext::CancelComposition() {
  if (!has_input_method_state_) {
    return;
  }

  QInputMethod* im = QGuiApplication::inputMethod();
  if (!im) {
    return;
  }

  im->reset();
}

void InputMethodContext::Commit() {
    
  if (!has_input_method_state_) {
    return;
  }

  QInputMethod* im = QGuiApplication::inputMethod();
  if (!im) {
    return;
  }

  im->commit(); 
}

void InputMethodContext::FocusedNodeChanged() {
  if (!client_) {
    return;
  }

  // Work around for https://launchpad.net/bugs/1323743
  if (QGuiApplication::focusWindow() &&
      QGuiApplication::focusWindow()->focusObject()) {
    QGuiApplication::focusWindow()->focusObjectChanged(
        QGuiApplication::focusWindow()->focusObject());
  }

  if (ShouldHideInputPanel() && client_->HasFocus()) {
    SetInputPanelVisibility(false);
  } else if (!has_input_method_state_ && ShouldShowInputPanel()) {
    // See https://launchpad.net/bugs/1400372
    SetInputPanelVisibility(true);
  } else if (has_input_method_state_ &&
             ime_bridge() &&
             ime_bridge()->focused_node_is_editable() &&
             QGuiApplication::inputMethod()) {
    QGuiApplication::inputMethod()->reset();
  }
}

InputMethodContext::InputMethodContext(InputMethodContextClient* client)
    : client_(client),
      has_input_method_state_(false) {
  QInputMethod* im = QGuiApplication::inputMethod();
  if (im) {
    connect(im, SIGNAL(visibleChanged()),
            SLOT(OnInputPanelVisibilityChanged()));
  }
}

InputMethodContext::~InputMethodContext() {
  QInputMethod* im = QGuiApplication::inputMethod();
  if (im) {
    im->disconnect(this);
  }
}

void InputMethodContext::DetachClient() {
  client_ = nullptr;
}

QVariant InputMethodContext::Query(Qt::InputMethodQuery query) const {
  switch (query) {
    case Qt::ImHints:
      if (!ime_bridge()) {
        return QVariant(QImHintsFromInputType(ui::TEXT_INPUT_TYPE_NONE));
      }
      return QVariant(QImHintsFromInputType(ime_bridge()->text_input_type()));
    case Qt::ImCursorRectangle: {
      if (!ime_bridge()) {
        return QRect();
      }
      // XXX: Is this in the right coordinate space?
      return QRect(ime_bridge()->caret_rect().x(),
                   ime_bridge()->caret_rect().y(),
                   ime_bridge()->caret_rect().width(),
                   ime_bridge()->caret_rect().height());
    }
    case Qt::ImCursorPosition:
      if (!ime_bridge()) {
        return 0;
      }
      return static_cast<int>(ime_bridge()->selection_cursor_position() &
                              INT_MAX);
    case Qt::ImSurroundingText:
      if (!ime_bridge()) {
        return QString();
      }
      return QString::fromStdString(
          base::UTF16ToUTF8(ime_bridge()->GetSelectionText()));
    case Qt::ImCurrentSelection:
      if (!ime_bridge()) {
        return QString();
      }
      return QString::fromStdString(
          base::UTF16ToUTF8(ime_bridge()->GetSelectedText()));
    case Qt::ImAnchorPosition:
      if (!ime_bridge()) {
        return 0;
      }
      return static_cast<int>(ime_bridge()->selection_anchor_position() &
                              INT_MAX);
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
    case Qt::ImTextBeforeCursor: {
      if (!ime_bridge()) {
        return QString();
      }
      std::string text = base::UTF16ToUTF8(ime_bridge()->GetSelectionText());
      return QString::fromStdString(
          text.substr(0, ime_bridge()->selection_cursor_position()));
    }
    case Qt::ImTextAfterCursor: {
      if (!ime_bridge()) {
        return QString();
      }
      std::string text = base::UTF16ToUTF8(ime_bridge()->GetSelectionText());
      if (ime_bridge()->selection_cursor_position() > text.length()) {
        return QString();
      }
      return QString::fromStdString(
          text.substr(ime_bridge()->selection_cursor_position(),
                      std::string::npos));
    }
#endif
    default:
      break;
  }

  return QVariant();
}

void InputMethodContext::FocusChanged(QFocusEvent* event) {
  if (!event->gotFocus()) {
    return;
  }

  if (!ShouldShowInputPanel()) {
    return;
  }

  SetInputPanelVisibility(true);
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
    if (ime_bridge()) {
      ime_bridge()->CommitText(base::UTF8ToUTF16(commit_string.toStdString()),
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
          qMin(attribute.start, (attribute.start + attribute.length)));
      selection_range.set_end(
          qMax(attribute.start, (attribute.start + attribute.length)));
      break;
    case QInputMethodEvent::TextFormat: {
      QTextCharFormat format =
          attribute.value.value<QTextFormat>().toCharFormat();
      blink::WebColor color = format.underlineColor().rgba();
      int start = qMin(attribute.start, (attribute.start + attribute.length));
      int end = qMax(attribute.start, (attribute.start + attribute.length));
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
                     cursor_position :
                     preedit_string.length());
  }

  if (ime_bridge()) {
    ime_bridge()->SetComposingText(
        base::UTF8ToUTF16(preedit_string.toStdString()),
        underlines, selection_range);
  }

  has_input_method_state_ = !preedit_string.isEmpty();
}

} // namespace qt
} // namespace oxide
