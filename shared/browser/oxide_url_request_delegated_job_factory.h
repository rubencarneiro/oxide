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

#ifndef _OXIDE_SHARED_BROWSER_URL_REQUEST_DELEGATED_JOB_FACTORY_H_
#define _OXIDE_SHARED_BROWSER_URL_REQUEST_DELEGATED_JOB_FACTORY_H_

#include <string>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "net/url_request/url_request_job_factory.h"

namespace oxide {

class BrowserContextIOData;

class URLRequestDelegatedJobFactory FINAL : public net::URLRequestJobFactory {
 public:
  URLRequestDelegatedJobFactory(
      scoped_ptr<net::URLRequestJobFactory> job_factory,
      BrowserContextIOData* context);
  ~URLRequestDelegatedJobFactory();

  static bool CanDelegateProtocol(const std::string& scheme);

 private:
  // net::URLRequestJobFactory implementation
  net::URLRequestJob* MaybeCreateJobWithProtocolHandler(
      const std::string& scheme,
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate) const FINAL;
  bool IsHandledProtocol(const std::string& scheme) const FINAL;
  bool IsHandledURL(const GURL& url) const FINAL;
  bool IsSafeRedirectTarget(const GURL& location) const FINAL;

  scoped_ptr<net::URLRequestJobFactory> job_factory_;
  BrowserContextIOData* context_;

  DISALLOW_COPY_AND_ASSIGN(URLRequestDelegatedJobFactory);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_URL_REQUEST_DELEGATED_JOB_FACTORY_H_
