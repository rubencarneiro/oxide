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

#include <map>
#include <queue>

#include "base/basictypes.h"
#include "base/memory/linked_ptr.h"
#include "url/gurl.h"

namespace oxide {

class WebFrame {
 public:
  WebFrame(int64 frame_id);
  virtual ~WebFrame();

  int64 identifier() const {
    return id_;
  }

  GURL url() const {
    return url_;
  }

  WebFrame* parent() const {
    return parent_;
  }

  void AddChildFrame(WebFrame* frame);
  void RemoveChildFrameWithID(int64 frame_id);

  void SetURL(const GURL& url);

  WebFrame* FindFrameWithIDInSubtree(int64 frame_id);

  std::vector<WebFrame *> GetChildFrames() const;

 private:
  typedef std::map<int64, linked_ptr<WebFrame> > ChildMap;

  void AddChildrenToQueue(std::queue<WebFrame *>* queue) const;
  WebFrame* GetChildFrameWithID(int64 frame_id) const;

  void SetParent(WebFrame* parent);

  virtual void OnParentChanged();
  virtual void OnChildAdded(WebFrame* child);
  virtual void OnChildRemoved(WebFrame* child);
  virtual void OnURLChanged();

  int64 id_;
  GURL url_;
  ChildMap child_frames_;
  WebFrame* parent_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebFrame);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_FRAME_H_
