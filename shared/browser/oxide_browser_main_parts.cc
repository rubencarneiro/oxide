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

#include "oxide_browser_main_parts.h"

#include "base/message_loop.h"

#include "shared/common/oxide_content_client.h"

#include "oxide_browser_process_main.h"
#include "oxide_content_browser_client.h"
#include "oxide_message_pump.h"

namespace oxide {

namespace {

base::MessagePump* CreateMessagePumpForUI() {
  return ContentClient::GetInstance()->browser()->
      CreateMessagePumpForUI();
}

} // namespace

BrowserMainParts::BrowserMainParts() {}

void BrowserMainParts::PreEarlyInitialization() {
  base::MessageLoop::InitMessagePumpForUIFactory(CreateMessagePumpForUI);
  main_message_loop_.reset(new base::MessageLoop(base::MessageLoop::TYPE_UI));
}

int BrowserMainParts::PreCreateThreads() {
  BrowserProcessMain::PreCreateThreads();
  return 0;
}

bool BrowserMainParts::MainMessageLoopRun(int* result_code) {
  MessageLoopForUI::current()->Start();
  return true;
}

} // namespace oxide
