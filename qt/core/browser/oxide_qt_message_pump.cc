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
#include "base/time/time.h"

QT_USE_NAMESPACE

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

  state_ = last_state;
}

void MessagePump::Quit() {
  CHECK(state_ && state_->event_loop) << "Called Quit() without calling Run()";

  state_->should_quit = true;
  state_->event_loop->exit();
}

void MessagePump::ScheduleWork() {
  if (work_scheduled_) {
    return;
  }

  work_scheduled_ = true;

  if (!state_) {
    // Handle being called before Start()
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

  if (work_scheduled_) {
    // Schedule events that might have already been posted
    PostWorkEvent();
  }
}

void MessagePump::timerEvent(QTimerEvent* event) {
  DCHECK(event->timerId() == delayed_work_timer_id_);

  CancelTimer();

  if (!state_) {
    ScheduleWork();
    return;
  }

  RunOneTask();
}

void MessagePump::customEvent(QEvent* event) {
  DCHECK(event->type() == GetChromiumEventType());

  work_scheduled_ = false;
  RunOneTask();
}

MessagePump::MessagePump()
    : work_scheduled_(false),
      delayed_work_timer_id_(0),
      state_(nullptr) {}

MessagePump::~MessagePump() {}

} // namespace qt
} // namespace oxide
