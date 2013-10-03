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

#ifndef _OXIDE_SHARED_BROWSER_MESSAGE_DISPATCHER_H_
#define _OXIDE_SHARED_BROWSER_MESSAGE_DISPATCHER_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/render_view_host_observer.h"

#include "shared/common/oxide_message_enums.h"

struct OxideMsg_SendMessage_Params;

namespace oxide {

class MessageHandler;
class OutgoingMessageRequest;
class WebFrame;
class WebView;

class MessageDispatcherBrowser FINAL : public content::RenderViewHostObserver {
 public:
  typedef std::vector<MessageHandler *> MessageHandlerVector;
  typedef std::vector<OutgoingMessageRequest *> OutgoingMessageRequestVector;

  struct V8Message {
    V8Message(WebView* view,
              MessageDispatcherBrowser* dispatcher,
              const OxideMsg_SendMessage_Params& params);

    WebFrame* frame;
    std::string world_id;
    int serial;
    std::string msg_id;
    std::string args;
    MessageDispatcherBrowser* dispatcher;
  };

  struct V8Response {
    V8Response(const OxideMsg_SendMessage_Params& params);

    bool IsError() const { return error != 0; }

    int error;
    std::string param;
  };

  MessageDispatcherBrowser(content::RenderViewHost* rvh);

  base::WeakPtr<MessageDispatcherBrowser> GetWeakPtr();
  content::RenderViewHost* GetRenderViewHost() const {
    return render_view_host();
  }

  bool OnMessageReceived(const IPC::Message& message) FINAL;

 private:
  void MaybeSendError(const OxideMsg_SendMessage_Params& params,
                      OxideMsg_SendMessage_Error::Value error_code,
                      const std::string& error_desc);
  void OnReceiveMessage(const OxideMsg_SendMessage_Params& params);

  base::WeakPtrFactory<MessageDispatcherBrowser> weak_factory_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(MessageDispatcherBrowser);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_MESSAGE_DISPATCHER_H_
