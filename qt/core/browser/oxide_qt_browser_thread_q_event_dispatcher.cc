// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include "oxide_qt_browser_thread_q_event_dispatcher.h"

#include <QCoreApplication>
#include <QEvent>
#include <QEventLoop>
#include <QSocketNotifier>
#include <QtDebug>
#include <QThread>
#include <QTimerEvent>

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/single_thread_task_runner.h"
#include "base/timer/timer.h"

namespace oxide {
namespace qt {

void BrowserThreadQEventDispatcher::IOWatcher::OnFileCanReadWithoutBlocking(
    int fd) {
  DCHECK_EQ(notifier_->socket(), fd);
  DCHECK_EQ(notifier_->type(), QSocketNotifier::Read);

  QEvent ev(QEvent::SockAct);
  QCoreApplication::sendEvent(notifier_, &ev);
}

void BrowserThreadQEventDispatcher::IOWatcher::OnFileCanWriteWithoutBlocking(
    int fd) {
  DCHECK_EQ(notifier_->socket(), fd);
  DCHECK_EQ(notifier_->type(), QSocketNotifier::Write);

  QEvent ev(QEvent::SockAct);
  QCoreApplication::sendEvent(notifier_, &ev);
}

BrowserThreadQEventDispatcher::IOWatcher::IOWatcher(
    QSocketNotifier* notifier)
    : notifier_(notifier) {}

BrowserThreadQEventDispatcher::IOWatcher::~IOWatcher() {}

BrowserThreadQEventDispatcher::TimerInstance::TimerInstance() {}
BrowserThreadQEventDispatcher::TimerInstance::~TimerInstance() {}

BrowserThreadQEventDispatcher::TimerData::TimerData()
    : interval(0), type(Qt::PreciseTimer), object(NULL) {}
BrowserThreadQEventDispatcher::TimerData::~TimerData() {}

BrowserThreadQEventDispatcher::SocketNotifierData::SocketNotifierData() {}
BrowserThreadQEventDispatcher::SocketNotifierData::~SocketNotifierData() {}

void BrowserThreadQEventDispatcher::AssertCalledOnValidThread() const {
  CHECK_EQ(thread(), QThread::currentThread()) <<
      "BrowserThreadQEventDispatcher accessed from the wrong thread!";
  CHECK(task_runner_->RunsTasksOnCurrentThread())
      << "BrowserThreadQEventDispatcher accessed from a thread other than "
      << "the one it runs tasks on. This check should only fail if the "
      << "event dispatcher is owned by the wrong thread";
}

void BrowserThreadQEventDispatcher::RunPostedTasks() {
  AssertCalledOnValidThread();

  {
    base::AutoLock lock(lock_);
    wakeup_task_posted_ = false;
  }

  QCoreApplication::sendPostedEvents();
}

void BrowserThreadQEventDispatcher::OnTimerExpired(
    const linked_ptr<TimerInstance>& timer_instance) {
  AssertCalledOnValidThread();

  std::set<int> ids = timer_instance->ids;
  for (std::set<int>::iterator it = ids.begin(); it != ids.end(); ++it) {
    int id = *it;
    if (timer_infos_.find(id) == timer_infos_.end()) {
      // A previous timer could have removed it
      continue;
    }

    QTimerEvent ev(id);
    QCoreApplication::sendEvent(timer_infos_[id].object, &ev);
  }

  std::list<linked_ptr<TimerInstance> >::iterator inst =
      std::find(timers_.begin(), timers_.end(), timer_instance);
  if (inst != timers_.end()) {
    timers_.erase(inst);
  }

  for (std::set<int>::iterator it = ids.begin(); it != ids.end(); ++it) {
    int id = *it;
    if (timer_infos_.find(id) == timer_infos_.end()) {
      continue;
    }

    timer_infos_[id].instance.reset();

    ScheduleTimer(id);
  }
}

void BrowserThreadQEventDispatcher::ScheduleTimer(int timer_id) {
  AssertCalledOnValidThread();

  DCHECK(timer_infos_.find(timer_id) != timer_infos_.end());

  TimerData& info = timer_infos_[timer_id];
  DCHECK(!info.instance.get());

  int interval = info.interval;
  Qt::TimerType type = info.type;

  // Timers less than 20ms are always precise
  if (interval <= 20) {
    type = Qt::PreciseTimer;
  }

  base::TimeDelta accuracy;

  switch (type) {
    case Qt::CoarseTimer:
      // CoarseTimer has an accuracy of +/-5%
      accuracy = base::TimeDelta::FromMilliseconds(interval / 20);
      break;
    case Qt::VeryCoarseTimer:
      // VeryCoarseTimer interval is rounded to the nearest second
      accuracy = base::TimeDelta::FromSeconds(1);
      interval = 1000 * ((interval + 500)/ 1000);
      break;
    default:
      break;
  }

  base::TimeTicks expected =
      base::TimeTicks::Now() + base::TimeDelta::FromMilliseconds(interval);

  linked_ptr<TimerInstance> timer_instance;

  // For CoarseTimer / VeryCoarseTimer, try to find an existing timer
  // instance to reuse
  if (accuracy > base::TimeDelta()) {
    base::TimeDelta last_difference = base::TimeDelta::Max();

    for (std::list<linked_ptr<TimerInstance> >::iterator it = timers_.begin();
         it != timers_.end(); ++it) {
      const linked_ptr<TimerInstance>& inst = *it;
      base::TimeTicks run_time = inst->timer->desired_run_time();

      if (run_time == expected) {
        // An exact match
        timer_instance = *it;
        break;
      }

      base::TimeDelta difference;
      if (run_time > expected) {
        difference = run_time - expected;
      } else {
        if (type == Qt::VeryCoarseTimer) {
          // For VeryCoarseTimer, only allow +1s
          continue;
        }
        difference = expected - run_time;
      }

      if (difference > last_difference) {
        break;
      }
      last_difference = difference;

      if (difference <= accuracy) {
        // This is a good enough match
        timer_instance = inst;
        break;
      }
    }
  }

  if (!timer_instance.get()) {
    // Create a new timer instance and insert it in to the list in order
    timer_instance = linked_ptr<TimerInstance>(new TimerInstance());
    timer_instance->timer.reset(new base::Timer(false, false));
    // As |this| owns the timer instance (via |timers_|), |this| is guaranteed
    // to be alive if timer callback runs
    timer_instance->timer->Start(
        FROM_HERE,
        base::TimeDelta::FromMilliseconds(interval),
        base::Bind(&BrowserThreadQEventDispatcher::OnTimerExpired,
                   base::Unretained(this),
                   timer_instance));

    std::list<linked_ptr<TimerInstance> >::iterator it = timers_.begin();
    while (it != timers_.end() &&
           (*it)->timer->desired_run_time() <
               timer_instance->timer->desired_run_time()) {
      ++it;
    }

    timers_.insert(it, timer_instance);
  }

  timer_instance->ids.insert(timer_id);
  info.instance = timer_instance;
}

bool BrowserThreadQEventDispatcher::processEvents(
    QEventLoop::ProcessEventsFlags flags) {
  AssertCalledOnValidThread();
  CHECK((flags & QEventLoop::EventLoopExec) == 0) <<
      "Using QEventLoop is not supported on threads created by Chromium!";
  CHECK((flags & QEventLoop::WaitForMoreEvents) == 0)
      << "Blocking threads created by Chromium to wait for Qt events is not "
      << "supported!";

  NOTIMPLEMENTED()
      << "QAbstractEventDispatcher::processEvents() is not implemented for "
      << "this thread";

  return false;
}

bool BrowserThreadQEventDispatcher::hasPendingEvents() {
  return false;
}

void BrowserThreadQEventDispatcher::registerSocketNotifier(
    QSocketNotifier *notifier) {
  AssertCalledOnValidThread();

  if (thread() != notifier->thread()) {
    qWarning() << "BrowserThreadQEventDispatcher: Notifier object belongs "
               << "to the wrong thread";
    return;
  }

  if (!notifier || notifier->socket() < 1 || !notifier->isEnabled()) {
    qWarning() << "BrowserThreadQEventDispatcher: Invalid socket notifier";
    return;
  }

  if (base::MessageLoop::current()->type() != base::MessageLoop::TYPE_IO) {
    qWarning() << "BrowserThreadQEventDispatcher: Cannot use QSocketNotifier "
               << "on this thread";
    return;
  }

  if (socket_notifiers_.find(notifier) != socket_notifiers_.end()) {
    qWarning() << "BrowserThreadQEventDispatcher:: SocketNotifier already "
               << "registered";
    return;
  }

  base::MessageLoopForIO::Mode mode;
  switch (notifier->type()) {
    case QSocketNotifier::Read:
      mode = base::MessageLoopForIO::WATCH_READ;
      break;
    case QSocketNotifier::Write:
      mode = base::MessageLoopForIO::WATCH_WRITE;
      break;
    case QSocketNotifier::Exception:
      qWarning() << "BrowserThreadQEventDispatcher: Cannot use "
                 << "QSocketNotifier::Exception";
      return;
    default:
      NOTREACHED();
      return;
  }

  linked_ptr<SocketNotifierData> notifier_data(new SocketNotifierData());
  notifier_data->watcher.reset(new IOWatcher(notifier));

  if (!base::MessageLoopForIO::current()->WatchFileDescriptor(
          notifier->socket(),
          true,
          mode,
          &notifier_data->controller,
          notifier_data->watcher.get())) {
    qWarning() << "BrowserThreadQEventDispatcher: Failed to watch file descriptor";
    return;
  }

  socket_notifiers_[notifier] = notifier_data;
}

void BrowserThreadQEventDispatcher::unregisterSocketNotifier(
    QSocketNotifier *notifier) {
  AssertCalledOnValidThread();

  if (!notifier) {
    return;
  }

  if (socket_notifiers_.find(notifier) == socket_notifiers_.end()) {
    return;
  }

  socket_notifiers_.erase(notifier);
}

void BrowserThreadQEventDispatcher::registerTimer(int timer_id,
                                                  int interval,
                                                  Qt::TimerType timer_type,
                                                  QObject *object) {
  AssertCalledOnValidThread();

  if (timer_id < 1 || interval < 0 || !object) {
    qWarning() << "BrowserThreadQEventDispatcher: Invalid timer parameters";
    return;
  }

  if (thread() != object->thread()) {
    qWarning() << "BrowserThreadQEventDispatcher: Timer object belongs to the "
               << "wrong thread";
    return;
  }

  if (timer_infos_.find(timer_id) != timer_infos_.end()) {
    qWarning() << "BrowserThreadQEventDispatcher: Timer with id " << timer_id
               << " is already registered";
    return;
  }

  TimerData info;
  info.interval = interval;
  info.type = timer_type;
  info.object = object;

  timer_infos_[timer_id] = info;

  ScheduleTimer(timer_id);
}

bool BrowserThreadQEventDispatcher::unregisterTimer(int timer_id) {
  AssertCalledOnValidThread();

  if (timer_id < 1) {
    return false;
  }

  if (timer_infos_.find(timer_id) == timer_infos_.end()) {
    return false;
  }

  linked_ptr<TimerInstance> timer_instance = timer_infos_[timer_id].instance;
  DCHECK(timer_instance.get());

  timer_instance->ids.erase(timer_id);

  if (timer_instance->ids.size() == 0) {
    std::list<linked_ptr<TimerInstance> >::iterator it =
        std::find(timers_.begin(), timers_.end(), timer_instance);
    DCHECK(it != timers_.end());

    timers_.erase(it);
  }

  timer_infos_.erase(timer_id);

  return true;
}

bool BrowserThreadQEventDispatcher::unregisterTimers(QObject *object) {
  AssertCalledOnValidThread();

  if (!object || timer_infos_.empty()) {
    return false;
  }

  std::vector<int> ids;
  for (std::map<int, TimerData>::iterator it = timer_infos_.begin();
       it != timer_infos_.end(); ++it) {
    if (it->second.object == object) {
      ids.push_back(it->first);
    }
  }

  for (std::vector<int>::iterator it = ids.begin(); it != ids.end(); ++it) {
    unregisterTimer(*it);
  }

  return true;
}

QList<QAbstractEventDispatcher::TimerInfo>
BrowserThreadQEventDispatcher::registeredTimers(
    QObject *object) const {
  AssertCalledOnValidThread();

  if (!object) {
    return QList<TimerInfo>();
  }

  QList<TimerInfo> result;

  for (std::map<int, TimerData>::const_iterator it = timer_infos_.begin();
       it != timer_infos_.end(); ++it) {
    if (it->second.object == object) {
      result.append(TimerInfo(it->first, it->second.interval, it->second.type));
    }
  }

  return result;
}

int BrowserThreadQEventDispatcher::remainingTime(int timer_id) {
  AssertCalledOnValidThread();

  if (timer_id < 1) {
    qWarning() << "BrowserThreadQEventDispatcher: Invalid timer id";
    return -1;
  }

  if (timer_infos_.find(timer_id) == timer_infos_.end()) {
    qWarning() << "BrowserThreadQEventDispatcher: Timer with id " << timer_id
               << " not found";
    return -1;
  }

  linked_ptr<TimerInstance> timer_instance = timer_infos_[timer_id].instance;
  DCHECK(timer_instance.get());

  base::TimeDelta remaining =
      timer_instance->timer->desired_run_time() - base::TimeTicks::Now();
  return remaining.InMilliseconds();
}

void BrowserThreadQEventDispatcher::wakeUp() {
  {
    base::AutoLock lock(lock_);
    if (wakeup_task_posted_) {
      return;
    }
    wakeup_task_posted_ = true;
  }

  // |this| is owned by the target thread's QThreadData. As long as the
  // target thread's QThreadData is not deleted until after the thread's
  // Chromium event loop has stopped processing events, then |this| will
  // always remain alive when processing an event on the target thread.
  // As QThreadData is deleted when the thread is torn down,
  // base::Unretained() is safe here as we can guarantee that |this| will
  // exist when running the task on the target thread.
  task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&BrowserThreadQEventDispatcher::RunPostedTasks,
                 base::Unretained(this)));
}

void BrowserThreadQEventDispatcher::interrupt() {
  wakeUp();
}

void BrowserThreadQEventDispatcher::flush() {}

BrowserThreadQEventDispatcher::BrowserThreadQEventDispatcher(
    const scoped_refptr<base::SingleThreadTaskRunner>& task_runner)
    : task_runner_(task_runner),
      wakeup_task_posted_(false) {
  DCHECK(task_runner_.get());
}

BrowserThreadQEventDispatcher::~BrowserThreadQEventDispatcher() {}

} // namespace qt
} // namespace oxide
