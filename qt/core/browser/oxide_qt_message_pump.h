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

#ifndef _OXIDE_QT_LIB_BROWSER_MESSAGE_PUMP_H_
#define _OXIDE_QT_LIB_BROWSER_MESSAGE_PUMP_H_

#include <QObject>
#include <QtGlobal>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/time/time.h"

#include "shared/browser/oxide_message_pump.h"

QT_BEGIN_NAMESPACE
class QEventLoop;
QT_END_NAMESPACE

QT_USE_NAMESPACE

namespace oxide {
namespace qt {

class MessagePump FINAL : public QObject,
                          public oxide::MessagePump {
 public:
  MessagePump();

  void Run(Delegate* delegate) FINAL;

  void Quit() FINAL;

  void ScheduleWork() FINAL;

  void ScheduleDelayedWork(const base::TimeTicks& delayed_work_time) FINAL;

  void Start(Delegate* delegate) FINAL;

 private:
  struct RunState {
    RunState() :
        delegate(NULL),
        event_loop(NULL),
        should_quit(false) {}

    Delegate* delegate;
    QEventLoop* event_loop;
    bool should_quit;
  };

  void timerEvent(QTimerEvent* event) FINAL;
  void customEvent(QEvent* event) FINAL;

  base::TimeTicks delayed_work_time_;
  int delayed_work_timer_id_;

  RunState* state_;
  RunState top_level_state_;

  DISALLOW_COPY_AND_ASSIGN(MessagePump);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_LIB_BROWSER_MESSAGE_PUMP_H_
