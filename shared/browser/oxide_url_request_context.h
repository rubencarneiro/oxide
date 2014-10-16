// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_URL_REQUEST_CONTEXT_H_
#define _OXIDE_SHARED_BROWSER_URL_REQUEST_CONTEXT_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"
#include "net/url_request/url_request_context_storage.h"

namespace base {

class SingleThreadTaskRunner;

} // namespace base

namespace oxide {

class BrowserContextIOData;

class URLRequestContext final : public net::URLRequestContext,
                                public base::SupportsWeakPtr<URLRequestContext> {
 public:
  URLRequestContext();
  ~URLRequestContext();

  net::URLRequestContextStorage* storage() { return &storage_; }

 private:
  net::URLRequestContextStorage storage_;

  DISALLOW_COPY_AND_ASSIGN(URLRequestContext);
};

class URLRequestContextGetter : public net::URLRequestContextGetter {
 public:
  virtual ~URLRequestContextGetter();

 protected:
  URLRequestContextGetter();

 private:
  scoped_refptr<base::SingleThreadTaskRunner> GetNetworkTaskRunner() const final;

  scoped_refptr<base::SingleThreadTaskRunner> network_task_runner_;

 private:
  DISALLOW_COPY_AND_ASSIGN(URLRequestContextGetter);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_URL_REQUEST_CONTEXT_H_
