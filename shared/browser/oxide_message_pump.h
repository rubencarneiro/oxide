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

#ifndef _OXIDE_BROWSER_MESSAGE_PUMP_H_
#define _OXIDE_BROWSER_MESSAGE_PUMP_H_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_pump.h"

namespace base {
class RunLoop;
}

namespace oxide {

class MessagePump : public base::MessagePump {
 public:
  MessagePump();
  ~MessagePump();

  virtual void Start(Delegate* delegate) = 0;

 protected:
  void SetupRunLoop();

 private:
  scoped_ptr<base::RunLoop> run_loop_;

  DISALLOW_COPY_AND_ASSIGN(MessagePump);
};

} // namespace oxide

#endif // _OXIDE_BROWSER_MESSAGE_PUMP_H_
