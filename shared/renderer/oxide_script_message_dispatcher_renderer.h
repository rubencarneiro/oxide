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

#ifndef _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_DISPATCHER_H_
#define _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_DISPATCHER_H_

#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/linked_ptr.h"
#include "content/public/renderer/render_frame_observer.h"
#include "v8/include/v8.h"

#include "shared/common/oxide_script_message_request.h"

struct OxideMsg_SendMessage_Params;

namespace blink {
class WebLocalFrame;
}

namespace oxide {

class ScriptMessageManager;

class ScriptMessageDispatcherRenderer final : public content::RenderFrameObserver {
 public:
  ScriptMessageDispatcherRenderer(content::RenderFrame* frame);
  ~ScriptMessageDispatcherRenderer();

  static ScriptMessageDispatcherRenderer* FromWebFrame(
      blink::WebLocalFrame* frame);

  linked_ptr<ScriptMessageManager> ScriptMessageManagerForWorldId(int world_id);

  void DidCreateScriptContext(v8::Handle<v8::Context> context,
                              int world_id);

 private:
  typedef std::vector<linked_ptr<ScriptMessageManager> > ScriptMessageManagerVector;

  void WillReleaseScriptContext(v8::Handle<v8::Context> context,
                                int world_id) final;
  bool OnMessageReceived(const IPC::Message& message) final;

  void OnReceiveMessage(const OxideMsg_SendMessage_Params& params);

  void ReturnError(ScriptMessageRequest::Error error,
                   const std::string& msg,
                   const OxideMsg_SendMessage_Params& orig);

  ScriptMessageManagerVector script_message_managers_;

  DISALLOW_COPY_AND_ASSIGN(ScriptMessageDispatcherRenderer);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_DISPATCHER_H_
