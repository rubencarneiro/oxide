// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_RENDERER_TOP_CONTROLS_HANDLER_H_
#define _OXIDE_SHARED_RENDERER_TOP_CONTROLS_HANDLER_H_

#include "base/macros.h"
#include "content/public/renderer/render_view_observer.h"
#include "third_party/WebKit/public/platform/WebTopControlsState.h"

namespace oxide {

class TopControlsHandler final : public content::RenderViewObserver {
 public:
  TopControlsHandler(content::RenderView* render_view);
  ~TopControlsHandler();

 private:
  void OnUpdateTopControlsState(blink::WebTopControlsState constraints,
                                blink::WebTopControlsState current,
                                bool animate);

  // IPC::Listener implementation
  bool OnMessageReceived(const IPC::Message& message) final;

  DISALLOW_COPY_AND_ASSIGN(TopControlsHandler);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_TOP_CONTROLS_HANDLER_H_
