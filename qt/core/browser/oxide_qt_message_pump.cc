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

#include "base/logging.h"
#include "base/message_loop/message_loop.h"

QT_USE_NAMESPACE

namespace oxide {
namespace qt {

namespace {

int GetChromiumEventType() {
  static int g_event_type = QEvent::registerEventType();
  return g_event_type;
}

int GetTimeIntervalMilliseconds(const base::TimeTicks& from) {
  int delay = static_cast<int>(
      ceil((from - base::TimeTicks::Now()).InMillisecondsF()));

  return delay < 0 ? 0 : delay;
}

} // namespace

void MessagePump::Run(base::MessagePump::Delegate* delegate) {
  QEventLoop event_loop;

  RunState state;
  state.delegate = delegate;
  state.event_loop = &event_loop;
  state.should_quit = false;

  RunState* last_state = state_;
  state_ = &state;

  event_loop.exec();

  state_ = last_state;
}

void MessagePump::Quit() {
  DCHECK(state_ && state_->event_loop) <<
      "Called Quit() without calling Run()";

  state_->should_quit = true;
  state_->event_loop->exit();
}

void MessagePump::ScheduleWork() {
  if (!state_) {
    // Handle being called before Start()
    return;
  }

  QCoreApplication::postEvent(
      this, new QEvent(QEvent::Type(GetChromiumEventType())));
}

void MessagePump::ScheduleDelayedWork(
    const base::TimeTicks& delayed_work_time) {
  if (delayed_work_time != delayed_work_time_) {
    delayed_work_time_ = delayed_work_time;
    killTimer(delayed_work_timer_id_);
    delayed_work_timer_id_ = 0;

    if (!delayed_work_time_.is_null()) {
      delayed_work_timer_id_ =
          startTimer(GetTimeIntervalMilliseconds(delayed_work_time_));
    }
  }
}

void MessagePump::OnStart() {
  DCHECK(!state_) <<
      "Called Start() more than once, or somebody already called Run()";

  top_level_state_.delegate = base::MessageLoop::current();
  state_ = &top_level_state_;
}

void MessagePump::timerEvent(QTimerEvent* event) {
  DCHECK(event->timerId() == delayed_work_timer_id_);

  // Clear the timer
  ScheduleDelayedWork(base::TimeTicks());

  base::TimeTicks next_delayed_work_time;
  state_->delegate->DoDelayedWork(&next_delayed_work_time);
  ScheduleDelayedWork(next_delayed_work_time);
}

void MessagePump::customEvent(QEvent* event) {
  DCHECK(event->type() == GetChromiumEventType());

  bool did_work = state_->delegate->DoWork();
  if (state_->should_quit) {
    return;
  }

  base::TimeTicks next_delayed_work_time;
  did_work |= state_->delegate->DoDelayedWork(&next_delayed_work_time);
  ScheduleDelayedWork(next_delayed_work_time);
  if (state_->should_quit) {
    return;
  }

  if (did_work) {
    ScheduleWork();
    return;
  }

  if (state_->delegate->DoIdleWork()) {
    ScheduleWork();
  }
}

MessagePump::MessagePump()
    : delayed_work_timer_id_(0),
      state_(nullptr) {}

MessagePump::~MessagePump() {}

} // namespace qt
} // namespace oxide
