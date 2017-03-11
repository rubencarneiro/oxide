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

#include <list>

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/threading/thread_task_runner_handle.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/render_process_host.h"

#include "oxide_browser_context.h"

namespace oxide {

namespace {

base::LazyInstance<std::list<BrowserContextDestroyer*>>::Leaky
    g_contexts_pending_deletion = LAZY_INSTANCE_INITIALIZER;

std::set<content::RenderProcessHost*> 
GetHostsForContext(BrowserContext* context) {
  std::set<content::RenderProcessHost*> hosts;

  for (auto it = content::RenderProcessHost::AllHostsIterator();
       !it.IsAtEnd(); it.Advance()) {
    content::RenderProcessHost* host = it.GetCurrentValue();
    if (host->GetBrowserContext() != context) {
      continue;
    }

    hosts.insert(host);
  }

  return std::move(hosts);
}

}

BrowserContextDestroyer::BrowserContextDestroyer(
    std::unique_ptr<BrowserContext> context,
    const std::set<content::RenderProcessHost*>& hosts,
    uint32_t otr_contexts_pending_deletion)
    : context_(std::move(context)),
      otr_contexts_pending_deletion_(otr_contexts_pending_deletion),
      finish_destroy_scheduled_(false) {
  DCHECK(hosts.size() > 0 ||
         (!context->IsOffTheRecord() &&
          (otr_contexts_pending_deletion > 0 ||
               context->HasOffTheRecordContext())));

  g_contexts_pending_deletion.Get().push_back(this);

  for (auto* host : hosts) {
    ObserveHost(host);
  }
}

BrowserContextDestroyer::~BrowserContextDestroyer() = default;

void BrowserContextDestroyer::ObserveHost(content::RenderProcessHost* host) {
  DCHECK(pending_host_ids_.find(host->GetID()) == pending_host_ids_.end());

  host->AddObserver(this);
  pending_host_ids_.insert(host->GetID());
}

void BrowserContextDestroyer::MaybeScheduleFinishDestroyContext(
    content::RenderProcessHost* host_being_destroyed) {
  DCHECK(!finish_destroy_scheduled_);

  if (pending_host_ids_.size() > 0) {
    // We're monitoring RenderProcessHosts that are using this context, so it's
    // not safe to delete yet
    return;
  }

  if (!context_->IsOffTheRecord() &&
      (otr_contexts_pending_deletion_ > 0 ||
           context_->HasOffTheRecordContext())) {
    // There are still live OTR BrowserContexts that depend on this context, so
    // it can't be deleted yet
    return;
  }

  finish_destroy_scheduled_ = true;

  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&BrowserContextDestroyer::FinishDestroyContext,
                 // We have exclusive ownership of |this| - nobody else can
                 // reference or delete it
                 base::Unretained(this)));
}

void BrowserContextDestroyer::FinishDestroyContext() {
  DCHECK(finish_destroy_scheduled_);
  CHECK_EQ(GetHostsForContext(context_.get()).size(), 0U)
      << "One or more RenderProcessHosts exist whilst its BrowserContext is "
      << "being deleted!";

  g_contexts_pending_deletion.Get().remove(this);

  if (context_->IsOffTheRecord()) {
    // If this is an OTR context and its owner BrowserContext has been scheduled
    // for deletion, update the owner's BrowserContextDestroyer
    BrowserContextDestroyer* orig_destroyer =
        GetForContext(context_->GetOriginalContext());
    if (orig_destroyer) {
      DCHECK_GT(orig_destroyer->otr_contexts_pending_deletion_, 0U);
      DCHECK(!orig_destroyer->finish_destroy_scheduled_);
      --orig_destroyer->otr_contexts_pending_deletion_;
      orig_destroyer->MaybeScheduleFinishDestroyContext();
    }
  }

  delete this;
}

// static
BrowserContextDestroyer* BrowserContextDestroyer::GetForContext(
    content::BrowserContext* context) {
  auto it = std::find_if(g_contexts_pending_deletion.Get().begin(),
                         g_contexts_pending_deletion.Get().end(),
                         [context](const BrowserContextDestroyer* d) {
    return d->context_.get() == context;
  });

  if (it == g_contexts_pending_deletion.Get().end()) {
    return nullptr;
  }

  return *it;
}

void BrowserContextDestroyer::RenderProcessHostDestroyed(
    content::RenderProcessHost* host) {
  DCHECK_GT(pending_host_ids_.size(), 0U);

  size_t erased = pending_host_ids_.erase(host->GetID());
  DCHECK_GT(erased, 0U);

  MaybeScheduleFinishDestroyContext(host);
}

// static
void BrowserContextDestroyer::DestroyContext(
    std::unique_ptr<BrowserContext> context) {

  bool is_otr_context = context->IsOffTheRecord();

  bool has_live_otr_context = false;
  uint32_t otr_contexts_pending_deletion = 0;

  if (!is_otr_context) {
    // If |context| is not an OTR BrowserContext, we need to keep track of how
    // many OTR BrowserContexts that were owned by it are scheduled for deletion
    // but still exist, as |context| must outlive these
    for (auto* destroyer : g_contexts_pending_deletion.Get()) {
      if (destroyer->context_->IsOffTheRecord() &&
          destroyer->context_->GetOriginalContext() == context.get()) {
        ++otr_contexts_pending_deletion;
      }
    }

    // If |context| is not an OTR BrowserContext but currently owns a live OTR
    // BrowserContext, then we have to outlive that
    has_live_otr_context = context->HasOffTheRecordContext();
  } else {
    // If |context| is an OTR BrowserContext and its owner has already been
    // scheduled for deletion, then we need to prevent the owner from being
    // deleted until after |context|
    BrowserContextDestroyer* orig_destroyer =
        GetForContext(context->GetOriginalContext());
    if (orig_destroyer) {
      CHECK(!orig_destroyer->finish_destroy_scheduled_);
      ++orig_destroyer->otr_contexts_pending_deletion_;
    }
  }

  // Get all of the live RenderProcessHosts that are using |context|
  std::set<content::RenderProcessHost*> hosts =
      GetHostsForContext(context.get());

  content::BrowserContext::NotifyWillBeDestroyed(context.get());

  // |hosts| might not be empty if the application released its BrowserContext
  // too early, or if |context| is an OTR context or this application is single
  // process

  if (!hosts.empty() ||
      otr_contexts_pending_deletion > 0 ||
      has_live_otr_context ||
      is_otr_context) {
    // |context| is not safe to delete yet. Note we always use
    // BrowserContextDestroyer for OTR contexts as if we don't then we'll leak
    // the owning BrowserContext if it's already been scheduled for deletion (as
    // we've already incremented otr_contexts_pending_deletion_)
    BrowserContextDestroyer* destroyer =
        new BrowserContextDestroyer(std::move(context),
                                    hosts,
                                    otr_contexts_pending_deletion);
    if (is_otr_context) {
      destroyer->MaybeScheduleFinishDestroyContext();
    }
  }
}

// static
void BrowserContextDestroyer::Shutdown() {
  auto destroy_all_unused_contexts = []() {
    auto it = g_contexts_pending_deletion.Get().begin();
    while (it != g_contexts_pending_deletion.Get().end()) {
      BrowserContextDestroyer* destroyer = *it;
      ++it;

      if (!destroyer->finish_destroy_scheduled_) {
        continue;
      }

      destroyer->FinishDestroyContext();
      // |destroyer| is invalid now
    }
  };

  // We make 2 passes over the list because the first pass can destroy an
  // incognito BrowserContext that subsequently schedules its owner context for
  // deletion
  destroy_all_unused_contexts();
  destroy_all_unused_contexts();
}

// static
void BrowserContextDestroyer::RenderProcessHostAssignedToSiteInstance(
    content::RenderProcessHost* host) {
  BrowserContextDestroyer* destroyer = GetForContext(host->GetBrowserContext());
  if (!destroyer) {
    return;
  }

  CHECK(!destroyer->finish_destroy_scheduled_);

  if (destroyer->pending_host_ids_.find(host->GetID()) !=
      destroyer->pending_host_ids_.end()) {
    return;
  }

  destroyer->ObserveHost(host);
}

} // namespace oxide
