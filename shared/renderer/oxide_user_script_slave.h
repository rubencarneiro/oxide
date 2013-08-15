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

#ifndef _OXIDE_SHARED_RENDERER_USER_SCRIPT_SLAVE_H_
#define _OXIDE_SHARED_RENDERER_USER_SCRIPT_SLAVE_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/shared_memory.h"
#include "content/public/renderer/render_process_observer.h"

#include "shared/common/oxide_user_script.h"

namespace WebKit {
class WebFrame;
}

namespace oxide {

class UserScript;

class UserScriptSlave FINAL : public content::RenderProcessObserver {
 public:
  typedef std::vector<linked_ptr<UserScript> > Vector;

  UserScriptSlave();
  ~UserScriptSlave();

  bool OnControlMessageReceived(const IPC::Message& message) FINAL;

  void OnRenderProcessShutdown() FINAL;

  void InjectScripts(WebKit::WebFrame* frame,
                     UserScript::RunLocation location);

 private:
  static int GetIsolatedWorldID(const std::string& name,
                                WebKit::WebFrame* frame);
  void OnUpdateUserScripts(base::SharedMemoryHandle handle);

  Vector user_scripts_;

  DISALLOW_COPY_AND_ASSIGN(UserScriptSlave);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_USER_SCRIPT_SLAVE_H_
