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

#ifndef _OXIDE_SHARED_BROWSER_WEB_FRAME_H_
#define _OXIDE_SHARED_BROWSER_WEB_FRAME_H_

#include <queue>
#include <vector>

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"
#include "ipc/ipc_sender.h"
#include "url/gurl.h"

#include "shared/browser/oxide_script_message_target.h"

namespace content {
class FrameTreeNode;
}

namespace oxide {

class ScriptMessageRequestImplBrowser;
class WebView;

// Represents a document frame in the renderer (a top-level frame or iframe).
// This is designed to be subclassed by each implementation. Each instance
// of this will typically own a publicly exposed webframe
class WebFrame : public ScriptMessageTarget {
 public:
  typedef std::vector<ScriptMessageRequestImplBrowser *> ScriptMessageRequestVector;

  void Destroy();

  static WebFrame* FromFrameTreeNode(content::FrameTreeNode* node);

  int64 FrameTreeNodeID() const;

  GURL url() const {
    return url_;
  }

  WebFrame* parent() const {
    return parent_;
  }

  WebView* view() const {
    return view_;
  }

  base::WeakPtr<WebFrame> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

  content::FrameTreeNode* frame_tree_node() const {
    return frame_tree_node_;
  }

  void SetURL(const GURL& url);
  void SetParent(WebFrame* parent);

  size_t ChildCount() const;
  WebFrame* ChildAt(size_t index) const;

  ScriptMessageRequestImplBrowser* SendMessage(const GURL& context,
                                               const std::string& msg_id,
                                               const std::string& args);
  bool SendMessageNoReply(const GURL& context,
                          const std::string& msg_id,
                          const std::string& args);

  const ScriptMessageRequestVector& current_script_message_requests() const {
    return current_script_message_requests_;
  }

 protected:
  WebFrame(content::FrameTreeNode* node, WebView* view);
  virtual ~WebFrame();

 private:
  friend class ScriptMessageRequestImplBrowser;
  typedef std::vector<WebFrame *> ChildVector;

  // ScriptMessageTarget
  virtual size_t GetScriptMessageHandlerCount() const OVERRIDE;
  virtual ScriptMessageHandler* GetScriptMessageHandlerAt(
      size_t index) const OVERRIDE;

  void AddChild(WebFrame* frame);
  void RemoveChild(WebFrame* frame);

  void AddScriptMessageRequest(ScriptMessageRequestImplBrowser* req);
  void RemoveScriptMessageRequest(ScriptMessageRequestImplBrowser* req);

  virtual void OnChildAdded(WebFrame* child);
  virtual void OnChildRemoved(WebFrame* child);
  virtual void OnURLChanged();

  content::FrameTreeNode* frame_tree_node_;
  GURL url_;
  ChildVector child_frames_;
  WebFrame* parent_;
  WebView* view_;
  int next_message_serial_;
  ScriptMessageRequestVector current_script_message_requests_;
  base::WeakPtrFactory<WebFrame> weak_factory_;
  bool destroyed_;

  DISALLOW_COPY_AND_ASSIGN(WebFrame);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_FRAME_H_
