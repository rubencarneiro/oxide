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

#include "oxide_web_frame_observer.h"

#include "content/public/renderer/render_view.h"
#include "third_party/WebKit/public/web/WebFrame.h"

#include "shared/common/oxide_messages.h"

namespace oxide {

WebFrameObserver::WebFrameObserver(content::RenderView* render_view) :
    content::RenderViewObserver(render_view) {}

WebFrameObserver::~WebFrameObserver() {}

void WebFrameObserver::FrameCreated(WebKit::WebFrame* parent,
                                    WebKit::WebFrame* frame) {
  render_view()->Send(
      new OxideHostMsg_FrameCreated(routing_id(),
                                    parent->identifier(),
                                    frame->identifier()));
}

void WebFrameObserver::FrameDetached(WebKit::WebFrame* frame) {
  render_view()->Send(
      new OxideHostMsg_FrameDetached(routing_id(),
                                     frame->identifier()));
}

} // namespace oxide
