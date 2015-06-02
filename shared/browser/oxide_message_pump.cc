// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/run_loop.h"
#include "base/threading/thread_local.h"

namespace oxide {

namespace {

base::LazyInstance<base::ThreadLocalPointer<MessagePump>> g_lazy_tls =
    LAZY_INSTANCE_INITIALIZER;

}

void MessagePump::WillProcessTask(const base::PendingTask& pending_task) {
  ++task_depth_;
}

void MessagePump::DidProcessTask(const base::PendingTask& pending_task) {
  --task_depth_;
  DCHECK_GE(task_depth_, 0);
}

// static
MessagePump* MessagePump::Get() {
  return g_lazy_tls.Pointer()->Get();
}

MessagePump::MessagePump()
    : task_depth_(0) {
  CHECK(!Get());
  g_lazy_tls.Pointer()->Set(this);
}

MessagePump::~MessagePump() {
  g_lazy_tls.Pointer()->Set(nullptr);
}

void MessagePump::Start() {
  base::MessageLoop* loop = base::MessageLoop::current();

  CHECK(!loop->is_running()) <<
      "Called Start() more than once or whilst inside a RunLoop";

  loop->AddTaskObserver(this);

  run_loop_.reset(new base::RunLoop());
  run_loop_->BeforeRun();

  OnStart();
}

void MessagePump::Stop() {
  base::MessageLoop* loop = base::MessageLoop::current();

  CHECK(loop->is_running() && run_loop_) <<
      "Called Stop() before calling Start()";
  CHECK_EQ(task_depth_, 0) << "Called Stop() inside a task";

  run_loop_->AfterRun();
  run_loop_.reset();

  loop->RemoveTaskObserver(this);
}

} // namespace oxide
