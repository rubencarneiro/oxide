// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_IN_PROCESS_RENDERER_OBSERVER_H_
#define _OXIDE_SHARED_BROWSER_IN_PROCESS_RENDERER_OBSERVER_H_

#include "base/macros.h"
#include "content/public/browser/render_process_host_observer.h"

namespace oxide {

class InProcessRendererObserver : public content::RenderProcessHostObserver {
 public:
  InProcessRendererObserver();
  ~InProcessRendererObserver() override;

 private:
  // content::RenderProcessHostObserver implementation
  void RenderProcessHostDestroyed(content::RenderProcessHost* host) override;

  DISALLOW_COPY_AND_ASSIGN(InProcessRendererObserver);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_IN_PROCESS_RENDERER_OBSERVER_H_
