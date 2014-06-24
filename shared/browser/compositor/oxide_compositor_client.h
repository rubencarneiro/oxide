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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_CLIENT_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_CLIENT_H_

#include "base/memory/scoped_ptr.h"

namespace oxide {

class CompositorFrameHandle;

class CompositorClient {
 public:
  virtual ~CompositorClient() {}

  virtual void CompositorDidCommit() = 0;

  virtual void CompositorSwapFrame(
      uint32 surface_id,
      scoped_ptr<CompositorFrameHandle> frame) = 0;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_CLIENT_H_
