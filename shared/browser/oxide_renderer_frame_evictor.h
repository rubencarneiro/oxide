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

#ifndef _OXIDE_SHARED_BROWSER_RENDERER_FRAME_EVICTOR_H_
#define _OXIDE_SHARED_BROWSER_RENDERER_FRAME_EVICTOR_H_

#include <list>
#include <map>

#include "base/compiler_specific.h"
#include "base/macros.h"

template <typename T> struct DefaultSingletonTraits;

namespace oxide {

class RendererFrameEvictorClient;

class RendererFrameEvictor final {
 public:
  static RendererFrameEvictor* GetInstance();
  ~RendererFrameEvictor();

  void AddFrame(RendererFrameEvictorClient* frame, bool locked);
  void RemoveFrame(RendererFrameEvictorClient* frame);
  void LockFrame(RendererFrameEvictorClient* frame);
  void UnlockFrame(RendererFrameEvictorClient* frame);

 private:
  friend struct DefaultSingletonTraits<RendererFrameEvictor>;
  RendererFrameEvictor();

  void CullUnlockedFrames();

  std::list<RendererFrameEvictorClient *> unlocked_frames_;
  std::map<RendererFrameEvictorClient *, size_t> locked_frames_;
  size_t max_number_of_saved_frames_;
  size_t max_number_of_handles_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_RENDERER_FRAME_EVICTOR_H_
