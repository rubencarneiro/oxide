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

#ifndef _OXIDE_SHARED_RENDERER_USER_SCRIPT_SCHEDULER_H_
#define _OXIDE_SHARED_RENDERER_USER_SCRIPT_SCHEDULER_H

#include <set>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "content/public/renderer/render_view_observer.h"

namespace blink {
class WebFrame;
}

namespace oxide {

class UserScriptScheduler : public content::RenderViewObserver {
 public:
  UserScriptScheduler(content::RenderView* render_view);

 private:
  void DoIdleInject();

  // content::RenderViewObserver implementation
  void DidFinishLoad(blink::WebLocalFrame* frame) override;
  void FrameDetached(blink::WebFrame* frame) override;

  bool idle_posted_;
  std::set<blink::WebLocalFrame *> pending_idle_frames_;
  base::WeakPtrFactory<UserScriptScheduler> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(UserScriptScheduler);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_USER_SCRIPT_SCHEDULER_H
