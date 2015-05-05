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

#include "oxide_redirection_intercept_throttle.h"

#include "base/memory/ref_counted.h"
#include "content/public/browser/resource_controller.h"
#include "net/url_request/redirect_info.h"

#include "oxide_browser_context.h"
#include "oxide_browser_context_delegate.h"

namespace oxide {

void RedirectionInterceptThrottle::WillRedirectRequest(
    const net::RedirectInfo& redirect_info,
    bool* defer) {
  BrowserContextIOData* context =
      BrowserContextIOData::FromResourceContext(resource_context_);
  if (!context) {
    return;
  }

  scoped_refptr<BrowserContextDelegate> delegate = context->GetDelegate();
  if (!delegate.get()) {
    return;
  }

  int rv = delegate->OnBeforeRedirect(request_, redirect_info.new_url);
  if (rv == net::ERR_ABORTED) {
    controller()->CancelAndIgnore();
  } else if (rv != net::OK) {
    controller()->CancelWithError(rv);
  }
}

const char* RedirectionInterceptThrottle::GetNameForLogging() const {
  return "Oxide_RedirectionInterceptThrottle";
}

RedirectionInterceptThrottle::RedirectionInterceptThrottle(
    net::URLRequest* request,
    content::ResourceContext* resource_context)
    : request_(request),
      resource_context_(resource_context) {}

RedirectionInterceptThrottle::~RedirectionInterceptThrottle() {}

} // namespace oxide
