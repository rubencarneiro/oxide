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

#include "oxide_browser_context_destroyer.h"

#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/render_process_host.h"

#include "oxide_browser_context.h"

namespace oxide {

BrowserContextDestroyer::BrowserContextDestroyer(
    BrowserContext* context,
    const std::set<content::RenderProcessHost*>& hosts)
    : context_(context),
      pending_hosts_(0) {
  for (std::set<content::RenderProcessHost*>::iterator it = hosts.begin();
       it != hosts.end(); ++it) {
    (*it)->AddObserver(this);
    ++pending_hosts_;
  }
}

BrowserContextDestroyer::~BrowserContextDestroyer() {}

void BrowserContextDestroyer::FinishDestroyContext() {
  DCHECK_EQ(pending_hosts_, 0U);

  delete context_;
  context_ = nullptr;

  delete this;
}

void BrowserContextDestroyer::RenderProcessHostDestroyed(
    content::RenderProcessHost* host) {
  DCHECK_GT(pending_hosts_, 0U);
  if (--pending_hosts_ != 0) {
    return;
  }

  if (content::RenderProcessHost::run_renderer_in_process()) {
    FinishDestroyContext();
  } else {
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&BrowserContextDestroyer::FinishDestroyContext,
                   // We have exclusive ownership of |this| - nobody else can
                   // reference or delete it
                   base::Unretained(this)));
  }
}

// static
void BrowserContextDestroyer::DestroyContext(BrowserContext* context) {
  CHECK(context->IsOffTheRecord() || !context->HasOffTheRecordContext());

  content::BrowserContext::NotifyWillBeDestroyed(context);

  std::set<content::RenderProcessHost*> hosts;

  for (content::RenderProcessHost::iterator it =
           content::RenderProcessHost::AllHostsIterator();
       !it.IsAtEnd(); it.Advance()) {
    content::RenderProcessHost* host = it.GetCurrentValue();
    if (host->GetBrowserContext() != context) {
      continue;
    }

    hosts.insert(host);
  }

  // XXX: Given that we shutdown the service worker context and that there
  // shouldn't be any live WebContents left, are there any circumstances
  // other than in single-process mode where |hosts| isn't empty?

  if (hosts.empty()) {
    delete context;
  } else {
    new BrowserContextDestroyer(context, hosts);
  }
}

} // namespace oxide
