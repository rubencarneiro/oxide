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

#ifndef _OXIDE_SHARED_RENDERER_MESSAGE_DISPATCHER_H_
#define _OXIDE_SHARED_RENDERER_MESSAGE_DISPATCHER_H_

#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/linked_ptr.h"
#include "content/public/renderer/render_view_observer.h"
#include "v8/include/v8.h"

struct OxideMsg_SendMessage_Params;

namespace WebKit {
class WebFrame;
}

namespace oxide {

class V8MessageManager;

class MessageDispatcherRenderer FINAL {
 public:

  // XXX: Not sure if this is really necessary - should we just use control
  //      (non-routed) messages for this, and use RenderProcessObserver
  //      instead?
  class EndPoint FINAL : public content::RenderViewObserver {
   public:
    EndPoint(content::RenderView* render_view);

    bool OnMessageReceived(const IPC::Message& message) FINAL;

   private:
    void OnReceiveMessage(const OxideMsg_SendMessage_Params& params);

    DISALLOW_IMPLICIT_CONSTRUCTORS(EndPoint);
  };

  MessageDispatcherRenderer();
  ~MessageDispatcherRenderer();

  void DidCreateScriptContext(blink::WebFrame* frame,
                              v8::Handle<v8::Context> context,
                              int world_id);

  void WillReleaseScriptContext(blink::WebFrame* frame,
                                v8::Handle<v8::Context> context,
                                int world_id);

 private:
  typedef std::vector<linked_ptr<V8MessageManager> > MessageManagerVector;

  void OnReceiveMessage(content::RenderView* render_view,
                        const OxideMsg_SendMessage_Params& params);

  MessageManagerVector message_managers_;

  DISALLOW_COPY_AND_ASSIGN(MessageDispatcherRenderer);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_MESSAGE_DISPATCHER_H_
