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

#include "oxide_url_request_delegated_job_factory.h"

#include "base/containers/hash_tables.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/strings/string_util.h"
#include "net/url_request/url_request_error_job.h"
#include "net/url_request/url_request_job_manager.h"
#include "url/gurl.h"

#include "oxide_browser_context.h"
#include "oxide_browser_context_delegate.h"
#include "oxide_url_request_delegated_job.h"

namespace oxide {

namespace {

const char kBlacklistedProtocols[][16] = {
  "about",
  "blob",
  "chrome",
  "chrome-devtools",
  "data",
  "file",
  "ftp",
  "http",
  "https",
  "ws",
  "wss"
};

}

net::URLRequestJob*
URLRequestDelegatedJobFactory::MaybeCreateJobWithProtocolHandler(
    const std::string& scheme,
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate) const {
  net::URLRequestJob* job =
      job_factory_->MaybeCreateJobWithProtocolHandler(scheme, request,
                                                      network_delegate);
  if (job) {
    return job;
  }

  if (net::URLRequestJobManager::GetInstance()->SupportsScheme(scheme)) {
    return NULL;
  }

  DCHECK(CanDelegateProtocol(scheme));

  scoped_refptr<BrowserContextDelegate> delegate(context_->GetDelegate());
  if (!delegate.get()) {
    return NULL;
  }

  job = delegate->CreateCustomURLRequestJob(request, network_delegate);
  if (job) {
    return job;
  }

  return new net::URLRequestErrorJob(request, network_delegate,
                                     net::ERR_FAILED);
}

bool URLRequestDelegatedJobFactory::IsHandledProtocol(
    const std::string& scheme) const {
  if (job_factory_->IsHandledProtocol(scheme)) {
    return true;
  }

  if (!CanDelegateProtocol(scheme)) {
    return false;
  }

  scoped_refptr<BrowserContextDelegate> delegate(context_->GetDelegate());
  if (!delegate.get()) {
    return false;
  }

  return delegate->IsCustomProtocolHandlerRegistered(scheme);
}

bool URLRequestDelegatedJobFactory::IsHandledURL(const GURL& url) const {
  if (job_factory_->IsHandledURL(url)) {
    return true;
  }

  std::string scheme = url.scheme();

  if (!CanDelegateProtocol(scheme)) {
    return false;
  }

  scoped_refptr<BrowserContextDelegate> delegate(context_->GetDelegate());
  if (!delegate.get()) {
    return false;
  }

  return delegate->IsCustomProtocolHandlerRegistered(scheme);
}

bool URLRequestDelegatedJobFactory::IsSafeRedirectTarget(
    const GURL& location) const {
  if (!location.is_valid()) {
    // This case is safely handled elsewhere
    return true;
  }

  if (!job_factory_->IsHandledProtocol(location.scheme())) {
    // Assume custom protocol handlers are never a safe redirect target
    return false;
  }

  return job_factory_->IsSafeRedirectTarget(location);
}

URLRequestDelegatedJobFactory::URLRequestDelegatedJobFactory(
    scoped_ptr<net::URLRequestJobFactory> job_factory,
    BrowserContextIOData* context)
    : job_factory_(job_factory.Pass()),
      context_(context) {
  DCHECK(job_factory_.get());
  DCHECK(context_);
}

URLRequestDelegatedJobFactory::~URLRequestDelegatedJobFactory() {}

// static
bool URLRequestDelegatedJobFactory::CanDelegateProtocol(
    const std::string& scheme) {
  static bool initialized = false;
  static base::hash_set<std::string> blacklisted_protocols;

  std::string lscheme(base::StringToLowerASCII(scheme));

  if (!initialized) {
    initialized = true;
    for (size_t i = 0; i < arraysize(kBlacklistedProtocols); ++i) {
      blacklisted_protocols.insert(kBlacklistedProtocols[i]);
    }
  }

  return blacklisted_protocols.find(lscheme) == blacklisted_protocols.end();
}

} // namespace oxide
