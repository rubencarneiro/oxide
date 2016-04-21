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

#ifndef _OXIDE_SHARED_BROWSER_WEB_FRAME_H_
#define _OXIDE_SHARED_BROWSER_WEB_FRAME_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "ipc/ipc_sender.h"
#include "url/gurl.h"

#include "shared/browser/oxide_script_message_target.h"
#include "shared/common/oxide_shared_export.h"

namespace base {
class Value;
}

namespace content {
class RenderFrameHost;
}

namespace oxide {

class ScriptMessageRequestImplBrowser;
class WebFrameTree;
class WebView;

// Represents a document frame (either the top level frame or an iframe). This
// tries to provide a representation of a view's frame tree whilst hiding
// the concept of RenderFrameHost, to make it suitable for a public API
class OXIDE_SHARED_EXPORT WebFrame : public ScriptMessageTarget {
 public:
  typedef std::vector<WebFrame*> ChildVector;
  typedef std::vector<ScriptMessageRequestImplBrowser*>
      ScriptMessageRequestVector;

  WebFrame(WebFrameTree* tree, content::RenderFrameHost* render_frame_host);

  ~WebFrame() override;

  // Find the WebFrame for |render_frame_host|
  static WebFrame* FromRenderFrameHost(
      content::RenderFrameHost* render_frame_host);

  // Return the last committed URL for this frame
  GURL GetURL() const;

  void AddChild(std::unique_ptr<WebFrame> child);

  void RemoveChild(WebFrame* child);

  // Return the parent frame
  WebFrame* parent() const { return parent_; }

  // Return the WebView that this frame is in. This can be null if the
  // corresponding WebContents hasn't been adopted by a WebView yet
  WebView* GetView() const;

  base::WeakPtr<WebFrame> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

  // Return the current RenderFrameHost for this frame
  content::RenderFrameHost* render_frame_host() const {
    return render_frame_host_;
  }

  // Notify this WebFrame that it's now being displayed by a new RFH
  void RenderFrameHostChanged(content::RenderFrameHost* render_frame_host);

  // Return the immediate children of this frame
  const ChildVector& GetChildFrames() const;

  // Send a message with |msg_id| and |payload| to the isolated world
  // addressed by |context|. Returns a request object on success with which you
  // can use to wait for a response
  std::unique_ptr<ScriptMessageRequestImplBrowser> SendMessage(
      const GURL& context,
      const std::string& msg_id,
      std::unique_ptr<base::Value> payload);

  // Send a message with |msg_id| and |payload| to the isolated world
  // addressed by |context|, for which you don't want a response. Returns
  // true on success
  bool SendMessageNoReply(const GURL& context,
                          const std::string& msg_id,
                          std::unique_ptr<base::Value> value);

  // Return the pending ScriptMessageRequests for this frame
  const ScriptMessageRequestVector& current_script_message_requests() const {
    return current_script_message_requests_;
  }

  // Remove |req| from the list of pending ScriptMessageRequests for this frame
  void RemoveScriptMessageRequest(ScriptMessageRequestImplBrowser* req);

  // XXX(chrisccoulson): We need to refactor ScriptMessageTarget so that
  //  this can die
  void set_script_message_target_delegate(ScriptMessageTarget* delegate) {
    script_message_target_delegate_ = delegate;
  }

 private:
  // ScriptMessageTarget implementation
  size_t GetScriptMessageHandlerCount() const override;
  const ScriptMessageHandler* GetScriptMessageHandlerAt(
      size_t index) const override;

  // The WebFrameTree that created us
  WebFrameTree* frame_tree_;

  // The current RenderFrameHost for this WebFrame
  content::RenderFrameHost* render_frame_host_;

  WebFrame* parent_;
  ChildVector child_frames_;

  // XXX(chrisccoulson): This should be on another object keyed off RFH
  ScriptMessageRequestVector current_script_message_requests_;

  // The message serial that will be used for the next outgoing script message
  int next_message_serial_;

  // The destructor for this WebFrame is running
  bool destroying_;

  ScriptMessageTarget* script_message_target_delegate_; // XXX: DIE

  base::WeakPtrFactory<WebFrame> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(WebFrame);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_FRAME_H_
