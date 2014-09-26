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

#ifndef _OXIDE_QT_CORE_BROWSER_BROWSER_THREAD_Q_EVENT_DISPATCHER_H_
#define _OXIDE_QT_CORE_BROWSER_BROWSER_THREAD_Q_EVENT_DISPATCHER_H_

#include <list>
#include <map>
#include <set>

#include <QAbstractEventDispatcher>
#include <Qt>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/synchronization/lock.h"

namespace base {
class SingleThreadTaskRunner;
class Timer;
}

namespace oxide {
namespace qt {

class BrowserThreadQEventDispatcher FINAL : public QAbstractEventDispatcher {
 public:
  BrowserThreadQEventDispatcher(
      const scoped_refptr<base::SingleThreadTaskRunner>& task_runner);
  ~BrowserThreadQEventDispatcher();

 private:
  void AssertCalledOnValidThread() const;

  struct TimerData;
  struct TimerInstance;

  void RunPostedTasks();
  void OnTimerExpired(const linked_ptr<TimerInstance>& timer_instance);

  void ScheduleTimer(int timer_id);

  // QAbstractEventDispatcher implementation
  bool processEvents(QEventLoop::ProcessEventsFlags flags) FINAL;
  bool hasPendingEvents() FINAL;

  void registerSocketNotifier(QSocketNotifier *notifier) FINAL;
  void unregisterSocketNotifier(QSocketNotifier *notifier) FINAL;

  void registerTimer(int timer_id,
                     int interval,
                     Qt::TimerType timer_type,
                     QObject *object) FINAL;
  bool unregisterTimer(int timer_id) FINAL;
  bool unregisterTimers(QObject *object) FINAL;
  QList<TimerInfo> registeredTimers(QObject *object) const FINAL;

  int remainingTime(int timer_id) FINAL;

  void wakeUp() FINAL;
  void interrupt() FINAL;
  void flush() FINAL;

  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  base::Lock lock_;
  bool wakeup_task_posted_;

  struct TimerInstance {
    TimerInstance();
    ~TimerInstance();

    scoped_ptr<base::Timer> timer;
    std::set<int> ids;
  };

  struct TimerData {
    TimerData();
    ~TimerData();

    int interval;
    Qt::TimerType type;
    QObject* object;
    linked_ptr<TimerInstance> instance;
  };

  std::map<int, TimerData> timer_infos_;
  std::list<linked_ptr<TimerInstance> > timers_;

  class IOWatcher : public base::MessageLoopForIO::Watcher {
   public:
    IOWatcher(QSocketNotifier* notifier);
    ~IOWatcher();

   private:
    void OnFileCanReadWithoutBlocking(int fd) FINAL;
    void OnFileCanWriteWithoutBlocking(int fd) FINAL;

    QSocketNotifier* notifier_;
  };

  struct SocketNotifierData {
    SocketNotifierData();
    ~SocketNotifierData();

    scoped_ptr<IOWatcher> watcher;
    base::MessageLoopForIO::FileDescriptorWatcher controller;
  };

  std::map<QSocketNotifier*, linked_ptr<SocketNotifierData> > socket_notifiers_;

  DISALLOW_COPY_AND_ASSIGN(BrowserThreadQEventDispatcher);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_BROWSER_THREAD_Q_EVENT_DISPATCHER_H_
