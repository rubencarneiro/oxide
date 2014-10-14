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

#ifndef _OXIDE_SHARED_BROWSER_USER_SCRIPT_MASTER_H_
#define _OXIDE_SHARED_BROWSER_USER_SCRIPT_MASTER_H_

#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"

namespace base {
class SharedMemory;
}

namespace content {
class RenderProcessHost;
}

namespace oxide {

class BrowserContext;
class UserScript;

class UserScriptMaster FINAL : public content::NotificationObserver {
 public:
  UserScriptMaster(BrowserContext* context);
  ~UserScriptMaster();

  void SerializeUserScriptsAndSendUpdates(std::vector<const UserScript *>& scripts);

  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) FINAL;

  static void ParseMetadata(UserScript* script);

 private:
  void SendUpdate(content::RenderProcessHost* process);

  BrowserContext* context_;
  scoped_ptr<base::SharedMemory> shmem_;
  content::NotificationRegistrar registrar_;

  DISALLOW_COPY_AND_ASSIGN(UserScriptMaster);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_USER_SCRIPT_MASTER_H_
