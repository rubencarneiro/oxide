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

#ifndef _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_DESTROYER_H_
#define _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_DESTROYER_H_

#include <set>

#include "base/macros.h"
#include "content/public/browser/render_process_host_observer.h"

namespace content {
class RenderProcessHost;
}

namespace oxide {

class BrowserContext;

class BrowserContextDestroyer : public content::RenderProcessHostObserver {
 public:
  static void DestroyContext(BrowserContext* context);

 private:
  BrowserContextDestroyer(BrowserContext* context,
                          const std::set<content::RenderProcessHost*>& hosts);
  ~BrowserContextDestroyer() override;

  void FinishDestroyContext();

  // content::RenderProcessHostObserver implementation
  void RenderProcessHostDestroyed(content::RenderProcessHost* host) override;

  BrowserContext* context_;

  uint32_t pending_hosts_;
  
  DISALLOW_COPY_AND_ASSIGN(BrowserContextDestroyer);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_DESTROYER_H_
