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

#include <vector>

#include "base/macros.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/shared_memory.h"
#include "content/public/renderer/render_process_observer.h"

#include "shared/common/oxide_user_script.h"

class GURL;

namespace blink {
class WebLocalFrame;
class WebScriptSource;
}

namespace oxide {

class UserScript;

class UserScriptSlave final : public content::RenderProcessObserver {
 public:
  static UserScriptSlave* GetInstance();

  UserScriptSlave();
  ~UserScriptSlave();

  void InjectScripts(blink::WebLocalFrame* frame,
                     UserScript::RunLocation location);

 private:
  typedef std::vector<linked_ptr<UserScript> > Vector;

  static int GetIsolatedWorldID(const GURL& url,
                                blink::WebLocalFrame* frame);
  void OnUpdateUserScripts(base::SharedMemoryHandle handle);

  void InjectGreaseMonkeyScriptInMainWorld(
      blink::WebLocalFrame* frame,
      const blink::WebScriptSource& script_source);

  // content::RenderProcessObserver implementation
  bool OnControlMessageReceived(const IPC::Message& message) final;
  void OnRenderProcessShutdown() final;

  bool render_process_shutting_down_;

  Vector user_scripts_;

  DISALLOW_COPY_AND_ASSIGN(UserScriptSlave);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_USER_SCRIPT_SLAVE_H_
