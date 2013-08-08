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

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_main_parts.h"

namespace base {
class MessageLoop;
}

namespace oxide {

class BrowserMainParts FINAL : public content::BrowserMainParts {
 public:
  BrowserMainParts();

  void PreEarlyInitialization() FINAL;

  int PreCreateThreads() FINAL;

  bool MainMessageLoopRun(int* result_code) FINAL;

  void PostDestroyThreads() FINAL;

 private:
  scoped_ptr<base::MessageLoop> main_message_loop_;

  DISALLOW_COPY_AND_ASSIGN(BrowserMainParts);
};

};

#endif // _OXIDE_SHARED_BROWSER_BROWSER_MAIN_PARTS_H_
