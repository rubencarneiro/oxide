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

#ifndef _OXIDE_SHARED_BROWSER_USER_SCRIPT_MASTER_H_
#define _OXIDE_SHARED_BROWSER_USER_SCRIPT_MASTER_H_

#include <set>
#include <vector>

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "components/keyed_service/core/keyed_service.h"

namespace base {
class SharedMemory;
}

namespace content {
class BrowserContext;
class RenderProcessHost;
}

namespace oxide {

class BrowserContext;
class UserScript;

class UserScriptMaster : public KeyedService {
 public:
  static UserScriptMaster* Get(content::BrowserContext* context);

  void SerializeUserScriptsAndSendUpdates(
      std::vector<const UserScript *>& scripts);

  static void ParseMetadata(UserScript* script);

  void RenderProcessCreated(content::RenderProcessHost* process);

 private:
  friend class UserScriptMasterFactory;
  typedef std::set<content::RenderProcessHost*> HostSet;

  UserScriptMaster(BrowserContext* context);
  ~UserScriptMaster();

  HostSet GetHostSet() const;
  void SendUpdate(content::RenderProcessHost* process);

  BrowserContext* context_;
  scoped_ptr<base::SharedMemory> shmem_;

  DISALLOW_COPY_AND_ASSIGN(UserScriptMaster);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_USER_SCRIPT_MASTER_H_
