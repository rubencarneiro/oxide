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

#ifndef _OXIDE_QT_LIB_BROWSER_WEB_FRAME_TREE_H_
#define _OXIDE_QT_LIB_BROWSER_WEB_FRAME_TREE_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"

#include "shared/browser/oxide_web_frame_tree.h"

namespace oxide {
namespace qt {

class WebFrameTree : public oxide::WebFrameTree {
 public:
  virtual ~WebFrameTree();

 protected:
  WebFrameTree(content::RenderViewHost* rvh);
};

class WebFrameTreeQQuick FINAL : public WebFrameTree {
 public:
  WebFrameTreeQQuick(content::RenderViewHost* rvh);
  virtual ~WebFrameTreeQQuick();

  oxide::WebFrame* CreateFrame() FINAL;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(WebFrameTreeQQuick);
};


} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_LIB_BROWSER_WEB_FRAME_TREE_H_
