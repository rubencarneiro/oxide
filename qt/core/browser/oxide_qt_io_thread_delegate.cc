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

#include "oxide_qt_io_thread_delegate.h"

#include <QPointer>
#include <QThread>

#include "base/lazy_instance.h"
#include "base/message_loop/message_loop_proxy.h"

#include "oxide_qt_browser_thread_q_event_dispatcher.h"

namespace oxide {
namespace qt {

namespace {
base::LazyInstance<QPointer<QThread> > g_io_thread;
}

void IOThreadDelegate::Init() {
  QThread* thread = QThread::currentThread();
  thread->setEventDispatcher(
      new BrowserThreadQEventDispatcher(base::MessageLoopProxy::current()));
  g_io_thread.Get() = thread;
}

void IOThreadDelegate::CleanUp() {}

IOThreadDelegate::IOThreadDelegate() {}

IOThreadDelegate::~IOThreadDelegate() {}

QThread* GetIOQThread() {
  return g_io_thread.Get();
}

} // namespace qt
} // namespace oxide
