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

#include "oxide_url_request_context.h"

#include "content/public/browser/browser_thread.h"

#include "oxide_io_thread.h"

namespace oxide {

URLRequestContext::URLRequestContext() :
    storage_(this) {
  set_net_log(IOThread::instance()->net_log());

  IOThread::Globals* io_thread_globals = IOThread::instance()->globals();
  set_host_resolver(io_thread_globals->host_resolver());
  set_cert_verifier(io_thread_globals->cert_verifier());
  set_http_auth_handler_factory(
      io_thread_globals->http_auth_handler_factory());
  set_proxy_service(io_thread_globals->proxy_service());
  set_throttler_manager(io_thread_globals->throttler_manager());
  set_cert_transparency_verifier(
      io_thread_globals->cert_transparency_verifier());
  set_ct_policy_enforcer(io_thread_globals->ct_policy_enforcer());
}

URLRequestContext::~URLRequestContext() {}

scoped_refptr<base::SingleThreadTaskRunner>
URLRequestContextGetter::GetNetworkTaskRunner() const {
  return network_task_runner_;
}

URLRequestContextGetter::URLRequestContextGetter() :
    network_task_runner_(content::BrowserThread::GetMessageLoopProxyForThread(
      content::BrowserThread::IO)) {}

URLRequestContextGetter::~URLRequestContextGetter() {}

} // namespace oxide
