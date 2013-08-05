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

#ifndef _OXIDE_SHARED_RENDERER_SCRIPT_EXECUTOR_H_
#define _OXIDE_SHARED_RENDERER_SCRIPT_EXECUTOR_H_

#include <map>
#include <queue>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/weak_ptr.h"
#include "content/public/renderer/render_view_observer.h"

#include "shared/common/oxide_messages.h"
#include "shared/common/oxide_user_script.h"

namespace WebKit {
class WebFrame;
}

namespace oxide {

class ScriptExecutor FINAL : public content::RenderViewObserver {
 public:
  ScriptExecutor(content::RenderView* render_view);

  void DidFinishDocumentLoad(WebKit::WebFrame* frame) FINAL;
  void DidFinishLoad(WebKit::WebFrame* frame) FINAL;
  void DidStartProvisionalLoad(WebKit::WebFrame* frame) FINAL;
  void DidCreateDocumentElement(WebKit::WebFrame* frame) FINAL;

  bool OnMessageReceived(const IPC::Message& message) FINAL;

 private:
  typedef std::queue<OxideMsg_ExecuteScript_Params> ExecutionQueue;
  typedef std::map<UserScript::RunLocation, ExecutionQueue> ExecutionMap;

  static int GetIsolatedWorldID(const std::string& name,
                                WebKit::WebFrame* frame);

  void OnExecuteScript(const OxideMsg_ExecuteScript_Params& params);

  void MaybeRun();
  void ExecuteScript(const OxideMsg_ExecuteScript_Params& params);
  void GetChildFrames(WebKit::WebFrame* frame,
                      std::vector<WebKit::WebFrame *>& frames);

  UserScript::RunLocation current_location_;
  ExecutionMap pending_execution_map_;
  base::WeakPtrFactory<ScriptExecutor> weak_factory_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(ScriptExecutor);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_SCRIPT_EXECUTOR_H_
