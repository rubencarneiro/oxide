// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#include "oxide_platform_run_loop.h"

#include "base/logging.h"
#include "base/run_loop.h"

namespace oxide {

PlatformRunLoop* PlatformRunLoop::s_top_run_loop_;

void PlatformRunLoop::WillProcessTask(const base::PendingTask& pending_task) {
  ++task_depth_;
}

void PlatformRunLoop::DidProcessTask(const base::PendingTask& pending_task) {
  --task_depth_;
  DCHECK_GE(task_depth_, 0);
}

PlatformRunLoop::PlatformRunLoop()
    : previous_run_loop_(nullptr),
      task_depth_(0) {}

PlatformRunLoop::~PlatformRunLoop() {
  Exit();
}

void PlatformRunLoop::Enter() {
  DCHECK(!run_loop_);

  previous_run_loop_ = s_top_run_loop_;
  s_top_run_loop_ = this;

  base::MessageLoop::current()->AddTaskObserver(this);

  run_loop_.reset(new base::RunLoop());
  run_loop_->BeforeRun();
}

void PlatformRunLoop::Exit() {
  if (!run_loop_) {
    return;
  }

  CHECK_EQ(s_top_run_loop_, this) <<
      "PlatformRunLoop::Exit called out of order";
  CHECK_EQ(task_depth_, 0) <<
      "PlatformRunLoop::Exit called during task processing";
  s_top_run_loop_ = previous_run_loop_;

  run_loop_->AfterRun();
  run_loop_.reset();

  base::MessageLoop::current()->RemoveTaskObserver(this);
}

} // namespace oxide
