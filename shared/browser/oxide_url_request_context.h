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

#ifndef _OXIDE_BROWSER_URL_REQUEST_CONTEXT_H_
#define _OXIDE_BROWSER_URL_REQUEST_CONTEXT_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/content_browser_client.h"
#include "net/url_request/url_request_context_getter.h"

namespace base {

class FilePath;
class SingleThreadTaskRunner;

} // namespace base

namespace net {
class URLRequestContext;
}

namespace oxide {

class URLRequestContextGetter : public net::URLRequestContextGetter {
 public:
  static URLRequestContextGetter* Create(
      content::ProtocolHandlerMap* protocol_handlers,
      base::FilePath data_path,
      base::FilePath cache_path,
      bool is_incognito);

  scoped_refptr<base::SingleThreadTaskRunner> GetNetworkTaskRunner() const FINAL;

 protected:
  URLRequestContextGetter();

  scoped_ptr<net::URLRequestContext> url_request_context_;

 private:
  DISALLOW_COPY_AND_ASSIGN(URLRequestContextGetter);
};

} // namespace oxide

#endif // _OXIDE_BROWSER_URL_REQUEST_CONTEXT_H_
