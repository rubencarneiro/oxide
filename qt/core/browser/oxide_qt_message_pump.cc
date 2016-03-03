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

#include "oxide_qt_message_pump.h"

#include <cmath>

#include <QCoreApplication>
#include <QEvent>
#include <QEventLoop>

#include "base/atomicops.h"
#include "base/logging.h"
#include "base/time/time.h"


namespace oxide {
namespace qt {

namespace {

QEvent::Type GetChromiumEventType() {
  static int g_event_type = QEvent::registerEventType();
  return QEvent::Type(g_event_type);
}

int GetTimeIntervalMilliseconds(const base::TimeTicks& from) {
  int delay = static_cast<int>(
      ceil((from - base::TimeTicks::Now()).InMillisecondsF()));

  return delay < 0 ? 0 : delay;
}

} // namespace

class MessagePump::RecursionHandler {
 public:
  RecursionHandler(RunState** state);
  ~RecursionHandler();

 private:
  RunState*& state() { return *state_; }

  RunState** state_;
  RunState* last_state_;
  RunState this_state_;
  PlatformRunLoop run_loop_;
};

MessagePump::RecursionHandler::RecursionHandler(RunState** s)
    : state_(s),
      last_state_(nullptr) {
  if (state()->running_task) {
    this_state_.delegate = state()->delegate;
    last_state_ = state();
    state() = &this_state_;
    run_loop_.Enter();
  }

  state()->running_task = true;
}

MessagePump::RecursionHandler::~RecursionHandler() {
  DCHECK(state()->running_task);
  state()->running_task = false;

  if (last_state_) {
    DCHECK_EQ(state(), &this_state_);
    state() = last_state_;
  }
}

void MessagePump::PostWorkEvent() {
  QCoreApplication::postEvent(this, new QEvent(GetChromiumEventType()));
}

void MessagePump::CancelTimer() {
  if (delayed_work_timer_id_ == 0) {
    return;
  }

  killTimer(delayed_work_timer_id_);
  delayed_work_timer_id_ = 0;
}

void MessagePump::RunOneTask() {
  DCHECK(state_);

  // Ensure we handle re-entry without a corresponding call to Run() (so we
  // don't have a nested RunLoop for this invocation yet). This likely means
  // we've been re-entered as a result of an external nested QEventLoop
  RecursionHandler recursion_handler(&state_);

  bool did_work = state_->delegate->DoWork();
  if (state_->should_quit) {
    return;
  }

  base::TimeTicks next_delayed_work_time;
  did_work |= state_->delegate->DoDelayedWork(&next_delayed_work_time);
  if (state_->should_quit) {
    return;
  }

  if (!next_delayed_work_time.is_null()) {
    ScheduleDelayedWork(next_delayed_work_time);
  }

  if (did_work) {
    ScheduleWork();
    return;
  }

  if (state_->delegate->DoIdleWork()) {
    ScheduleWork();
  }
}

void MessagePump::Run(base::MessagePump::Delegate* delegate) {
  QEventLoop event_loop;

  RunState state;
  state.delegate = delegate;
  state.event_loop = &event_loop;
  state.should_quit = false;

  RunState* last_state = state_;
  state_ = &state;

  event_loop.exec();

  DCHECK_EQ(state_, &state);
  state_ = last_state;
}

void MessagePump::Quit() {
  CHECK(state_ && state_->event_loop) << "Called Quit() without calling Run()";

  state_->should_quit = true;
  state_->event_loop->exit();
}

void MessagePump::ScheduleWork() {
  // ScheduleWork can be called from any thread
  if (base::subtle::NoBarrier_CompareAndSwap(&work_scheduled_, 0, 1)) {
    return;
  }

  PostWorkEvent();
}

void MessagePump::ScheduleDelayedWork(
    const base::TimeTicks& delayed_work_time) {
  CancelTimer();
  int interval = GetTimeIntervalMilliseconds(delayed_work_time);
  if (interval > 0) {
    delayed_work_timer_id_ = startTimer(interval);
  } else {
    ScheduleWork();
  }
}

void MessagePump::OnStart() {
  top_level_state_.delegate = base::MessageLoop::current();
  state_ = &top_level_state_;

  if (base::subtle::NoBarrier_Load(&work_scheduled_)) {
    // Post an event for work that's already been scheduled
    PostWorkEvent();
  }
}

void MessagePump::timerEvent(QTimerEvent* event) {
  DCHECK(event->timerId() == delayed_work_timer_id_);

  CancelTimer();

  if (!state_) {
    // Start() hasn't been called yet. Post an event to retry
    ScheduleWork();
    return;
  }

  RunOneTask();
}

void MessagePump::customEvent(QEvent* event) {
  DCHECK(event->type() == GetChromiumEventType());

  if (!state_) {
    // Start() hasn't been called yet. Returning here means that OnStart()
    // will post an event to run scheduled work
    return;
  }

  base::subtle::NoBarrier_Store(&work_scheduled_, 0);
  RunOneTask();
}

MessagePump::MessagePump()
    : work_scheduled_(0),
      delayed_work_timer_id_(0),
      state_(nullptr) {}

MessagePump::~MessagePump() {}

} // namespace qt
} // namespace oxide
