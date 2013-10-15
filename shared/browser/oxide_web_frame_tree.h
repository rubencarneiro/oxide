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

#ifndef _OXIDE_SHARED_BROWSER_WEB_FRAME_TREE_H_
#define _OXIDE_SHARED_BROWSER_WEB_FRAME_TREE_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/public/browser/render_view_host_observer.h"

namespace oxide {

class WebFrame;
class WebView;

class WebFrameTree : public content::RenderViewHostObserver {
 public:
  virtual ~WebFrameTree();

  static WebFrameTree* FromRenderViewHost(content::RenderViewHost* rvh);

  WebFrame* GetRootFrame();
  WebView* GetView() const;
  content::RenderViewHost* GetRenderViewHost() const;

  void RenderViewHostDestroyed(content::RenderViewHost* rvh) FINAL;
  bool OnMessageReceived(const IPC::Message& message) FINAL;

 protected:
  WebFrameTree(content::RenderViewHost* rvh);

 private:
  void OnFrameCreated(int64 parent_frame_id, int64 frame_id);
  void OnFrameDetached(int64 frame_id);

  virtual WebFrame* CreateFrame() = 0;

  WebFrame* root_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebFrameTree);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_FRAME_TREE_H_
