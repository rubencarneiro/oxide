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

#include "base/message_loop/message_pump_x11.h"

#include "shared/browser/oxide_content_browser_client.h"
#include "shared/common/oxide_content_client.h"

#if defined(USE_X11)
Display* base::MessagePumpX11::GetDefaultXDisplay() {
  return oxide::ContentClient::GetInstance()->browser()->GetDefaultXDisplay();
}
#endif
