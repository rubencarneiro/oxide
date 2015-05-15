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

#include "oxide_message_pump.h"

#include "base/logging.h"
#include "base/run_loop.h"

namespace oxide {

void MessagePump::SetupRunLoop() {
  run_loop_.reset(new base::RunLoop());
  run_loop_->BeforeRun();
}

void MessagePump::Stop() {
  CHECK_EQ(task_depth_, 0) <<
      "Stopping Oxide whilst dispatching a task from the event queue is bad!";
  run_loop_->AfterRun();
}

void MessagePump::WillProcessTask(const base::PendingTask& pending_task) {
  ++task_depth_;
}

void MessagePump::DidProcessTask(const base::PendingTask& pending_task) {
  --task_depth_;
  DCHECK(task_depth_ >= 0);
}

MessagePump::MessagePump() :
    task_depth_(0) {
  // FIXME(chrisccoulson): There's no MessageLoop for this thread just yet
  //base::MessageLoop::current()->AddTaskObserver(this);
}

MessagePump::~MessagePump() {}

} // namespace oxide
