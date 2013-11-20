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
#include "url/gurl.h"

#include "shared/browser/oxide_message_target.h"

namespace oxide {

class OutgoingMessageRequest;
class WebView;

// Represents a document frame in the renderer (a top-level frame or iframe).
// This is designed to be subclassed by each implementation. Each instance
// of this will typically own a publicly exposed webframe
class WebFrame : public MessageTarget {
 public:
  // Use this to delete a WebFrame rather than calling the destructor, so
  // that we can remove the frame from its parent before the derived destructor
  // is called
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

  WebView* view() const {
    return view_;
  }
  void set_view(WebView* view) {
    view_ = view;
  }

  base::WeakPtr<WebFrame> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

  void SetURL(const GURL& url);
  void SetParent(WebFrame* parent);

  size_t ChildCount() const;
  WebFrame* ChildAt(size_t index) const;

  void AddChildrenToQueue(std::queue<WebFrame *>* queue) const;

  bool SendMessage(const std::string& world_id,
                   const std::string& msg_id,
                   const std::string& args,
                   OutgoingMessageRequest* req);
  bool SendMessageNoReply(const std::string& world_id,
                          const std::string& msg_id,
                          const std::string& args);

  virtual size_t GetMessageHandlerCount() const OVERRIDE;
  virtual MessageHandler* GetMessageHandlerAt(size_t index) const OVERRIDE;

  virtual size_t GetOutgoingMessageRequestCount() const;
  virtual OutgoingMessageRequest* GetOutgoingMessageRequestAt(size_t index) const;

 protected:
  WebFrame();
  virtual ~WebFrame();

 private:
  typedef std::vector<WebFrame *> ChildVector;

  void AddChildFrame(WebFrame* frame);
  void RemoveChildFrame(WebFrame* frame);

  virtual void OnChildAdded(WebFrame* child);
  virtual void OnChildRemoved(WebFrame* child);
  virtual void OnURLChanged();

  int64 id_;
  GURL url_;
  ChildVector child_frames_;
  WebFrame* parent_;
  WebView* view_;
  int next_message_serial_;
  bool destroyed_;
  base::WeakPtrFactory<WebFrame> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(WebFrame);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_FRAME_H_
