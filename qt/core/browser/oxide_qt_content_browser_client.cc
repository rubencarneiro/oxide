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
#include <QThread>

#include "oxide_qt_browser_thread_q_event_dispatcher.h"
#include "oxide_qt_location_provider.h"

namespace oxide {
namespace qt {

ContentBrowserClient::ContentBrowserClient() {}

content::LocationProvider*
ContentBrowserClient::OverrideSystemLocationProvider() {
  // Give the geolocation thread a Qt event dispatcher, so that we can use
  // Queued signals / slots between it and the IO thread
  QThread* thread = QThread::currentThread();
  if (!thread->eventDispatcher()) {
    thread->setEventDispatcher(
      new BrowserThreadQEventDispatcher(base::MessageLoopProxy::current()));
  }

  return new LocationProvider();
}

} // namespace qt
} // namespace oxide
