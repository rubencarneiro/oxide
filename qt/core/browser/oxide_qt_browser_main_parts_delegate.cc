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

#include "oxide_qt_browser_main_parts_delegate.h"

#include "base/memory/scoped_ptr.h"

#include "oxide_qt_io_thread_delegate.h"
#include "oxide_qt_message_pump.h"

namespace oxide {
namespace qt {

namespace {

scoped_ptr<base::MessagePump> CreateMessagePumpForUI() {
  return make_scoped_ptr(new MessagePump()).PassAs<base::MessagePump>();
}

}

oxide::IOThread::Delegate* BrowserMainPartsDelegate::GetIOThreadDelegate() {
  return new IOThreadDelegate();
}

oxide::BrowserMainParts::Delegate::MessagePumpFactory*
BrowserMainPartsDelegate::GetMessagePumpFactory() {
  return CreateMessagePumpForUI;
}

BrowserMainPartsDelegate::BrowserMainPartsDelegate() {}

BrowserMainPartsDelegate::~BrowserMainPartsDelegate() {}

} // namespace qt
} // namespace oxide
