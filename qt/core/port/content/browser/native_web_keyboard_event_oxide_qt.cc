// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include "content/public/browser/native_web_keyboard_event.h"

#include <QKeyEvent>

#include "base/logging.h"

namespace content {

namespace {

QKeyEvent* CopyEvent(QKeyEvent* event) {
  if (!event) {
    return NULL;
  }

  return new QKeyEvent(*event);
}

}

NativeWebKeyboardEvent::NativeWebKeyboardEvent()
    : os_event(NULL),
      skip_in_browser(false),
      match_edit_command(false) {}

NativeWebKeyboardEvent::NativeWebKeyboardEvent(gfx::NativeEvent native_event)
    : os_event(NULL),
      skip_in_browser(false),
      match_edit_command(false) {
  NOTREACHED();
}

NativeWebKeyboardEvent::NativeWebKeyboardEvent(
    ui::EventType key_event_type,
    bool is_char,
    wchar_t character,
    int state,
    double time_stamp_seconds)
    : os_event(NULL),
      skip_in_browser(false),
      match_edit_command(false) {
  NOTREACHED();
}

NativeWebKeyboardEvent::NativeWebKeyboardEvent(
    const NativeWebKeyboardEvent& other)
    : WebKeyboardEvent(other),
      os_event(CopyEvent(other.os_event)),
      skip_in_browser(other.skip_in_browser),
      match_edit_command(false) {
}

NativeWebKeyboardEvent::~NativeWebKeyboardEvent() {
  delete os_event;
}

NativeWebKeyboardEvent& NativeWebKeyboardEvent::operator=(
    const NativeWebKeyboardEvent& other) {
  WebKeyboardEvent::operator=(other);
  delete os_event;
  os_event = CopyEvent(other.os_event);
  skip_in_browser = other.skip_in_browser;
  match_edit_command = other.match_edit_command;
  return *this;
}

} // namespace content
