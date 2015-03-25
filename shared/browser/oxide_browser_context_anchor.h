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

#ifndef _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_ANCHOR_H_
#define _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_ANCHOR_H_

#include "base/macros.h"
#include "content/public/browser/render_process_host_observer.h"

template <typename T> struct DefaultSingletonTraits;

namespace content {
class RenderProcessHost;
}

namespace oxide {

class BrowserContextAnchor : public content::RenderProcessHostObserver {
 public:
  static BrowserContextAnchor* GetInstance();

  void RenderProcessWillLaunch(content::RenderProcessHost* host);

 private:
  friend struct DefaultSingletonTraits<BrowserContextAnchor>;

  BrowserContextAnchor();

  // content::RenderProcessHostObserver implementation
  void RenderProcessHostDestroyed(content::RenderProcessHost* host) override;

  DISALLOW_COPY_AND_ASSIGN(BrowserContextAnchor);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_ANCHOR_H_
