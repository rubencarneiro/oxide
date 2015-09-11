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

#ifndef _OXIDE_QT_CORE_BROWSER_MESSAGE_PUMP_H_
#define _OXIDE_QT_CORE_BROWSER_MESSAGE_PUMP_H_

#include <QObject>
#include <QtGlobal>

#include "base/macros.h"

#include "shared/browser/oxide_message_pump.h"

QT_BEGIN_NAMESPACE
class QEventLoop;
QT_END_NAMESPACE


namespace base {
class TimeTicks;
}

namespace oxide {
namespace qt {

class MessagePump : public QObject,
                    public oxide::MessagePump {
 public:
  MessagePump();
  ~MessagePump() override;

 private:
  void PostWorkEvent();
  void CancelTimer();
  void RunOneTask();

  // base::MessagePump implementation
  void Run(Delegate* delegate) override;
  void Quit() override;
  void ScheduleWork() override;
  void ScheduleDelayedWork(const base::TimeTicks& delayed_work_time) override;

  // oxide::MessagePump implementation
  void OnStart() override;

  // QObject implementation
  void timerEvent(QTimerEvent* event) override;
  void customEvent(QEvent* event) override;

  struct RunState {
    RunState() :
        delegate(nullptr),
        event_loop(nullptr),
        should_quit(false) {}

    Delegate* delegate;
    QEventLoop* event_loop;
    bool should_quit;
  };

  int32_t work_scheduled_;

  int delayed_work_timer_id_;

  RunState* state_;
  RunState top_level_state_;

  DISALLOW_COPY_AND_ASSIGN(MessagePump);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_MESSAGE_PUMP_H_
