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

#include "oxide_lifecycle_observer.h"

#include "base/bind.h"
#include "base/callback.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/storage_partition.h"
#include "net/cookies/cookie_store.h"

#include "oxide_browser_context.h"
#include "oxide_browser_platform_integration.h"

namespace oxide {

namespace {

void FlushStoragePartition(content::StoragePartition* partition) {
  partition->Flush();
}

void FlushBrowserContext(BrowserContext* context) {
  context->GetCookieStore()->FlushStore(base::Closure());

  content::BrowserContext::ForEachStoragePartition(
      context, base::Bind(&FlushStoragePartition));
}

}

void LifecycleObserver::ApplicationStateChanged() {
  if (BrowserPlatformIntegration::GetInstance()->GetApplicationState() !=
      BrowserPlatformIntegration::APPLICATION_STATE_SUSPENDED) {
    return;
  }

  BrowserContext::ForEach(base::Bind(&FlushBrowserContext));
}

LifecycleObserver::LifecycleObserver() {}

LifecycleObserver::~LifecycleObserver() {}

} // namespace oxide
