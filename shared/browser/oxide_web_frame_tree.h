// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_WEB_FRAME_TREE_H_
#define _OXIDE_SHARED_BROWSER_WEB_FRAME_TREE_H_

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

namespace oxide {

class WebFrame;
class WebFrameTreeObserver;

// This class manages a tree of WebFrames
class WebFrameTree : public content::WebContentsUserData<WebFrameTree>,
                     public content::WebContentsObserver {
 public:
  ~WebFrameTree() override;

  WebFrame* root_frame() const { return root_frame_.get(); }

  typedef base::Callback<bool(WebFrame*)> ForEachFrameCallback;
  void ForEachFrame(const ForEachFrameCallback& callback);

 private:
  friend class content::WebContentsUserData<WebFrameTree>;
  friend class WebFrame;
  friend class WebFrameTreeObserver;

  WebFrameTree(content::WebContents* contents);

  void WebFrameRemoved(WebFrame* frame);

  void AddObserver(WebFrameTreeObserver* observer);
  void RemoveObserver(WebFrameTreeObserver* observer);

  // content::WebContentsObserver implementation
  void RenderFrameCreated(content::RenderFrameHost* render_frame_host) override;
  void RenderFrameHostChanged(content::RenderFrameHost* old_host,
                              content::RenderFrameHost* new_host) override;
  void FrameDeleted(content::RenderFrameHost* render_frame_host) override;
  void DidCommitProvisionalLoadForFrame(
      content::RenderFrameHost* render_frame_host,
      const GURL& url,
      ui::PageTransition transition_type) override;

  base::ObserverList<WebFrameTreeObserver> observers_;

  // This should be torn down before |observers_|
  scoped_ptr<WebFrame> root_frame_;

  DISALLOW_COPY_AND_ASSIGN(WebFrameTree);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_FRAME_TREE_H_
