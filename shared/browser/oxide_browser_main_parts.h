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

#ifndef _OXIDE_SHARED_BROWSER_BROWSER_MAIN_PARTS_H_
#define _OXIDE_SHARED_BROWSER_BROWSER_MAIN_PARTS_H_

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_main_parts.h"

#include "shared/browser/oxide_io_thread.h"

namespace base {
class MessageLoop;
class MessagePump;
}

namespace gfx {
class Screen;
}

namespace oxide {

class BrowserMainParts FINAL : public content::BrowserMainParts {
 public:

  class Delegate {
   public:
    virtual ~Delegate();

    typedef scoped_ptr<base::MessagePump> (MessagePumpFactory)();

    virtual IOThread::Delegate* GetIOThreadDelegate();
    virtual MessagePumpFactory* GetMessagePumpFactory() = 0;
  };

  BrowserMainParts(Delegate* delegate);
  ~BrowserMainParts();

 private:
  // content::BrowserMainParts implementation
  virtual void PreEarlyInitialization() FINAL;
  virtual int PreCreateThreads() FINAL;
  virtual void PreMainMessageLoopRun() FINAL;
  virtual bool MainMessageLoopRun(int* result_code) FINAL;
  virtual void PostMainMessageLoopRun() FINAL;
  virtual void PostDestroyThreads() FINAL;

  scoped_ptr<Delegate> delegate_;

  scoped_ptr<base::MessageLoop> main_message_loop_;
  scoped_ptr<IOThread> io_thread_;
  scoped_ptr<gfx::Screen> primary_screen_;

  DISALLOW_COPY_AND_ASSIGN(BrowserMainParts);
};

} // namespace oxide

#endif //_OXIDE_SHARED_BROWSER_BROWSER_MAIN_PARTS_H_
