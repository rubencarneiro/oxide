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

#ifndef _OXIDE_SHARED_BROWSER_PLATFORM_RUN_LOOP_H_
#define _OXIDE_SHARED_BROWSER_PLATFORM_RUN_LOOP_H_

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"

#include "shared/common/oxide_shared_export.h"

namespace base {
class RunLoop;
}

namespace oxide {

// Helper class for managing base::RunLoop that corresponds to a platform event
// loop created and executed outside of Oxide. The purpose of this is for
// managing the RunLoop depth on the UI thread MessageLoop, for correct
// handling of nested tasks. One of these is created at applicaton startup, and
// then new ones should be created whenever we detect nested pumping of the
// MessagePump that didn't come from an internal use of base::RunLoop
class OXIDE_SHARED_EXPORT PlatformRunLoop
    : public base::MessageLoop::TaskObserver {
 public:
  PlatformRunLoop();
  ~PlatformRunLoop();

  void Enter();
  void Exit();

 private:
  // base::MessageLoop::TaskObserver implementation
  void WillProcessTask(const base::PendingTask& pending_task) override;
  void DidProcessTask(const base::PendingTask& pending_task) override;

  scoped_ptr<base::RunLoop> run_loop_;

  PlatformRunLoop* previous_run_loop_;
  static PlatformRunLoop* s_top_run_loop_;

  int task_depth_;

  DISALLOW_COPY_AND_ASSIGN(PlatformRunLoop);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_PLATFORM_RUN_LOOP_H_
