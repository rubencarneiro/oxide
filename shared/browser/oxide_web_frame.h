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
#include "base/memory/linked_ptr.h"
#include "base/memory/weak_ptr.h"
#include "url/gurl.h"

#include "shared/browser/oxide_message_dispatcher_browser.h"

namespace content {
class RenderViewHost;
}

namespace oxide {

class OutgoingMessageRequest;
class WebFrameTree;
class WebView;

// Represents a document frame in the renderer (a top-level frame or iframe).
// This is designed to be subclassed by each implementation. Each instance
// of this will typically own a publicly exposed webframe
class WebFrame {
 public:
  virtual ~WebFrame();

  void DestroyFrame();

  int64 identifier() const {
    return id_;
  }
  void set_identifier(int64 id) {
    id_ = id;
  }

  GURL url() const {
    return url_;
  }

  WebFrame* parent() const {
    return parent_;
  }

  void set_tree(WebFrameTree* tree) {
    tree_ = tree;
  }

  WebView* GetView() const;
  content::RenderViewHost* GetRenderViewHost() const;

  base::WeakPtr<WebFrame> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

  void SetURL(const GURL& url);
  void SetParent(WebFrame* parent);

  WebFrame* FindFrameWithID(int64 frame_id) const;

  size_t ChildCount() const;
  WebFrame* ChildAt(size_t index) const;

  bool SendMessage(const std::string& world_id,
                   const std::string& msg_id,
                   const std::string& args,
                   OutgoingMessageRequest* req);
  bool SendMessageNoReply(const std::string& world_id,
                          const std::string& msg_id,
                          const std::string& args);

  virtual MessageDispatcherBrowser::MessageHandlerVector
      GetMessageHandlers() const;
  virtual MessageDispatcherBrowser::OutgoingMessageRequestVector
      GetOutgoingMessageRequests() const;

 protected:
  WebFrame();

 private:
  typedef std::vector<linked_ptr<WebFrame> > ChildVector;

  void AddChildFrame(WebFrame* frame);
  void RemoveChildFrame(WebFrame* frame);

  void AddChildrenToQueue(std::queue<WebFrame *>* queue) const;

  virtual void OnChildAdded(WebFrame* child);
  virtual void OnChildRemoved(WebFrame* child);
  virtual void OnURLChanged();

  int64 id_;
  GURL url_;
  ChildVector child_frames_;
  WebFrame* parent_;
  WebFrameTree* tree_;
  int next_message_serial_;
  base::WeakPtrFactory<WebFrame> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(WebFrame);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_FRAME_H_
