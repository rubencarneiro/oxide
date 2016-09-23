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

#include <memory>
#include <set>

#include "base/macros.h"
#include "content/public/browser/render_process_host_observer.h"

namespace content {
class BrowserContext;
class RenderProcessHost;
}

namespace oxide {

class BrowserContext;

// A mechanism to manage BrowserContext destruction, ensuring it stays alive
// until consumers inside Chromium no longer require it
class BrowserContextDestroyer : public content::RenderProcessHostObserver {
 public:
  // Schedule |context| for deletion. If no RenderProcessHosts are using it then
  // this will result in it being deleted immediately, else deletion will be
  // happen after all RenderProcessHosts using it have gone away
  static void DestroyContext(std::unique_ptr<BrowserContext> context);

  // Delete all BrowserContexts that are pending deletion and safe to be deleted
  static void Shutdown();

  // Notify that |host| has been assigned to a SiteInstance. This is the first
  // notification we get from content after a RenderProcessHost is created,
  // although this doesn't mean it was actually just created.
  // This ensures that |host| will be tracked if its BrowserContext has already
  // been scheduled for deletion.
  static void RenderProcessHostAssignedToSiteInstance(
      content::RenderProcessHost* host);

 private:
  BrowserContextDestroyer(std::unique_ptr<BrowserContext> context,
                          const std::set<content::RenderProcessHost*>& hosts,
                          uint32_t otr_contexts_pending_deletion);
  ~BrowserContextDestroyer() override;

  void ObserveHost(content::RenderProcessHost* host);

  void MaybeScheduleFinishDestroyContext(
      content::RenderProcessHost* host_being_destroyed = nullptr);
  void FinishDestroyContext();

  static BrowserContextDestroyer* GetForContext(
      content::BrowserContext* context);

  // content::RenderProcessHostObserver implementation
  void RenderProcessHostDestroyed(content::RenderProcessHost* host) override;

  std::unique_ptr<BrowserContext> context_;

  std::set<int> pending_host_ids_;

  uint32_t otr_contexts_pending_deletion_;

  bool finish_destroy_scheduled_;
  
  DISALLOW_COPY_AND_ASSIGN(BrowserContextDestroyer);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_DESTROYER_H_
