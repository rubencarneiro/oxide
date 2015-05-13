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

#include "oxide_browser_context_anchor.h"

#include "base/memory/singleton.h"
#include "content/public/browser/render_process_host.h"

#include "oxide_browser_context.h"

namespace oxide {

BrowserContextAnchor::BrowserContextAnchor() {}

void BrowserContextAnchor::RenderProcessHostDestroyed(
    content::RenderProcessHost* host) {
  host->RemoveObserver(this);
  BrowserContext::FromContent(host->GetBrowserContext())->Release();

  size_t removed = tracked_render_process_hosts_.erase(host->GetID());
  DCHECK_GT(removed, 0U);
}

// static
BrowserContextAnchor* BrowserContextAnchor::GetInstance() {
  return Singleton<BrowserContextAnchor>::get();
}

void BrowserContextAnchor::RenderProcessWillLaunch(
    content::RenderProcessHost* host) {
  int id = host->GetID();
  if (tracked_render_process_hosts_.find(id) !=
      tracked_render_process_hosts_.end()) {
    return;
  }

  tracked_render_process_hosts_.insert(id);

  BrowserContext::FromContent(host->GetBrowserContext())->AddRef();
  host->AddObserver(this);
}

} // namespace oxide
