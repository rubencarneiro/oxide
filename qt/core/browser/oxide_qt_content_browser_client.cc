// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#include "oxide_qt_content_browser_client.h"

#include <QList>
#include <QTouchDevice>

#include "base/lazy_instance.h"

#include "oxide_qt_message_pump.h"
#include "oxide_qt_web_preferences.h"

namespace oxide {
namespace qt {

namespace {
base::LazyInstance<WebPreferences> g_default_web_preferences =
    LAZY_INSTANCE_INITIALIZER;
}

ContentBrowserClient::ContentBrowserClient() {}

base::MessagePump* ContentBrowserClient::CreateMessagePumpForUI() {
  return new MessagePump();
}

oxide::WebPreferences* ContentBrowserClient::GetDefaultWebPreferences() {
  return g_default_web_preferences.Pointer();
}

bool ContentBrowserClient::IsTouchSupported() {
  // XXX: Is there a way to get notified if a touch device is added?
  return QTouchDevice::devices().size() > 0;
}

} // namespace qt
} // namespace oxide
