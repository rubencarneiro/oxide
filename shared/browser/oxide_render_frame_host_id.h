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

#ifndef _OXIDE_SHARED_BROWSER_RENDER_FRAME_HOST_ID_H_
#define _OXIDE_SHARED_BROWSER_RENDER_FRAME_HOST_ID_H_

namespace content {
class RenderFrameHost;
}

namespace oxide {

// Helper class that allows a reference to a RenderFrameHost to be stored
// safely without having to implement WebContentsObserver (if you don't need
// notification of the RFH being deleted) or without risking a dangling
// pointer
class RenderFrameHostID {
 public:
  RenderFrameHostID();

  static RenderFrameHostID FromHost(content::RenderFrameHost* host);

  // Return the RenderFrameHost for this ID. Will return nullptr if the RFH
  // no longer exists
  content::RenderFrameHost* ToHost() const;

  bool IsValid() const;

  bool operator==(const RenderFrameHostID& other) const;
  bool operator!=(const RenderFrameHostID& other) const;

  // Allows this to be used as a key in std::map
  bool operator<(const RenderFrameHostID& other) const;

 private:
  int render_process_id_;
  int render_frame_id_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_RENDER_FRAME_HOST_ID_H_
