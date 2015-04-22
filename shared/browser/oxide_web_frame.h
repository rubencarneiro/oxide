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

#include <vector>

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "ipc/ipc_sender.h"
#include "url/gurl.h"

#include "shared/browser/oxide_script_message_target.h"

namespace content {
class RenderFrameHost;
}

namespace oxide {

class ScriptMessageRequestImplBrowser;
class WebView;

// Represents a document frame in the renderer (a top-level frame or iframe).
// This is designed to be subclassed by each implementation. Each instance
// of this will typically own a publicly exposed WebFrame
class WebFrame : public ScriptMessageTarget {
 public:
  typedef std::vector<ScriptMessageRequestImplBrowser*>
      ScriptMessageRequestVector;

  WebFrame(content::RenderFrameHost* render_frame_host,
           WebView* view);

  // Find the WebFrame for |frame_tree_node_id|. Currently only used in
  // WebView::OpenURLFromTab - please don't add new code which uses this
  static WebFrame* FromFrameTreeNodeID(int64 frame_tree_node_id);

  // Find the WebFrame for |render_frame_host|
  static WebFrame* FromRenderFrameHost(
      content::RenderFrameHost* render_frame_host);

  // Correctly destroy |frame|. Once this function returns, |frame| will
  // be invalid
  static void Destroy(WebFrame* frame);

  // Return the last committed URL for this frame
  GURL GetURL() const;

  // Initialize the parent of this frame to |parent|. This is not a constructor
  // parameter because it calls in to the parents subclass, which may require
  // the child frame to be fully constructed
  void InitParent(WebFrame* parent);

  // Return the parent frame
  WebFrame* parent() const { return parent_; }

  // Return the WebView that this frame is in
  WebView* view() const { return view_.get(); }

  base::WeakPtr<WebFrame> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

  // Return the current RenderFrameHost for this frame
  content::RenderFrameHost* render_frame_host() const {
    return render_frame_host_;
  }

  // Set the active RenderFrameHost for this WebFrame
  void SetRenderFrameHost(content::RenderFrameHost* render_frame_host);

  // Return the number of immediate children of this frame
  size_t GetChildCount() const;

  // Return the frame at |index|
  WebFrame* GetChildAt(size_t index) const;

  // Send a message with |msg_id| and payload |args| to the isolated world
  // addressed by |context|. Returns a request object on success with which you
  // can use to wait for a response
  scoped_ptr<ScriptMessageRequestImplBrowser> SendMessage(
      const GURL& context,
      const std::string& msg_id,
      const std::string& args);

  // Send a message with |msg_id| and payload |args| to the isolated world
  // addressed by |context|, for which you don't want a response. Returns
  // true on success
  bool SendMessageNoReply(const GURL& context,
                          const std::string& msg_id,
                          const std::string& args);

  // Return the pending ScriptMessageRequests for this frame
  const ScriptMessageRequestVector& current_script_message_requests() const {
    return current_script_message_requests_;
  }

  // Remove |req| from the list of pending ScriptMessageRequests for this frame
  void RemoveScriptMessageRequest(ScriptMessageRequestImplBrowser* req);

  // Notify this frame that a new URL was committed
  virtual void DidCommitNewURL();

 protected:
  virtual ~WebFrame();

 private:
  typedef std::vector<WebFrame *> ChildVector;

  // Notify this WebFrame that it's about to be deleted. This allows
  // it to destroy its children before the destructor in the derived class
  // is called, which will typically then destroy its publicly exposed
  // WebFrame
  void WillDestroy();

  // Delete this WebFrame. Called after WillDestroyFrame and can be overridden
  // by the implementation
  virtual void Delete();

  // Add |child| to this frame, calling OnChildAdded
  void AddChild(WebFrame* child);

  // Remove |child| from this frame, calling OnChildRemoved
  void RemoveChild(WebFrame* child);

  // ScriptMessageTarget implementatin
  size_t GetScriptMessageHandlerCount() const override;
  const ScriptMessageHandler* GetScriptMessageHandlerAt(
      size_t index) const override;

  // Notify the subclass that |child| was added to this frame
  virtual void OnChildAdded(WebFrame* child);

  // Notify the subclass that |child| has been removed from this frame
  virtual void OnChildRemoved(WebFrame* child);

  WebFrame* parent_;
  base::WeakPtr<WebView> view_;
  content::RenderFrameHost* render_frame_host_;
  ChildVector child_frames_;
  ScriptMessageRequestVector current_script_message_requests_;

  // The message serial that will be used for the next outgoing script message
  int next_message_serial_;

  // Whether WillDestroy() has been called on this frame
  bool destroyed_;

  base::WeakPtrFactory<WebFrame> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(WebFrame);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_FRAME_H_
