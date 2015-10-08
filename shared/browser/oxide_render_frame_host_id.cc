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

#include "oxide_render_frame_host_id.h"

#include "base/logging.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"

namespace oxide {

RenderFrameHostID::RenderFrameHostID()
    : render_process_id_(-1),
      render_frame_id_(-1) {}

// static
RenderFrameHostID RenderFrameHostID::FromHost(content::RenderFrameHost* host) {
  RenderFrameHostID id;
  id.render_process_id_ = host->GetProcess()->GetID();
  id.render_frame_id_ = host->GetRoutingID();
  return id;
}

content::RenderFrameHost* RenderFrameHostID::ToHost() const {
  DCHECK(IsValid());
  return content::RenderFrameHost::FromID(render_process_id_,
                                          render_frame_id_);
}

bool RenderFrameHostID::IsValid() const {
  return render_process_id_ != -1 && render_frame_id_ != -1;
}

bool RenderFrameHostID::operator==(const RenderFrameHostID& other) const {
  return render_process_id_ == other.render_process_id_ &&
         render_frame_id_ == other.render_frame_id_;
}

bool RenderFrameHostID::operator!=(const RenderFrameHostID& other) const {
  return !(*this == other);
}

bool RenderFrameHostID::operator<(const RenderFrameHostID& other) const {
  if (render_process_id_ < other.render_process_id_) {
    return true;
  }

  if (render_process_id_ != other.render_process_id_) {
    return false;
  }

  if (render_frame_id_ < other.render_frame_id_) {
    return true;
  }

  return false;
}

} // namespace oxide
