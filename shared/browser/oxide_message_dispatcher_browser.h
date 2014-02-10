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

#ifndef _OXIDE_SHARED_BROWSER_MESSAGE_DISPATCHER_H_
#define _OXIDE_SHARED_BROWSER_MESSAGE_DISPATCHER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/public/browser/browser_message_filter.h"

namespace oxide {

class MessageDispatcherBrowser FINAL : public content::BrowserMessageFilter {
 public:
  MessageDispatcherBrowser(int render_process_id);
  ~MessageDispatcherBrowser();

  bool OnMessageReceived(const IPC::Message& message,
                         bool* message_was_ok) FINAL;
 private:
  const int render_process_id_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(MessageDispatcherBrowser);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_MESSAGE_DISPATCHER_H_
