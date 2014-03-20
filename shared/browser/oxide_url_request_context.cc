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

#include <algorithm>

#include "base/logging.h"
#include "content/public/browser/browser_thread.h"

#include "oxide_browser_context.h"
#include "oxide_io_thread_globals.h"

namespace oxide {

class URLRequestContextFactory {
 public:
  URLRequestContextFactory() {}
  virtual ~URLRequestContextFactory() {}

  virtual void Initialize(scoped_ptr<URLRequestContext> context) = 0;
};

namespace {

class MainURLRequestContextFactory : public URLRequestContextFactory {
 public:
  MainURLRequestContextFactory(
      BrowserContextIOData* context,
      content::ProtocolHandlerMap* protocol_handlers,
      content::ProtocolHandlerScopedVector protocol_interceptors) :
      context_(context),
      protocol_interceptors_(protocol_interceptors.Pass()) {
    std::swap(protocol_handlers_, *protocol_handlers);
  }

  void Initialize(scoped_ptr<URLRequestContext> request_context) FINAL {
    context_->Init(request_context.Pass(),
                   protocol_handlers_,
                   protocol_interceptors_.Pass());
  }

 private:
  BrowserContextIOData* context_;
  content::ProtocolHandlerMap protocol_handlers_;
  content::ProtocolHandlerScopedVector protocol_interceptors_;
};

} // namespace

URLRequestContext::URLRequestContext() :
    storage_(this) {}

URLRequestContext::~URLRequestContext() {}

URLRequestContextGetter::URLRequestContextGetter(
    URLRequestContextFactory* factory) :
    factory_(factory) {}

net::URLRequestContext* URLRequestContextGetter::GetURLRequestContext() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

  if (!url_request_context_) {
    DCHECK(factory_);

    scoped_ptr<URLRequestContext> context(new URLRequestContext());

    IOThreadGlobals* io_thread_globals = IOThreadGlobals::GetInstance();
    context->set_net_log(io_thread_globals->net_log());
    context->set_host_resolver(io_thread_globals->host_resolver());
    context->set_cert_verifier(io_thread_globals->cert_verifier());
    context->set_http_auth_handler_factory(
        io_thread_globals->http_auth_handler_factory());
    context->set_proxy_service(io_thread_globals->proxy_service());
    context->set_throttler_manager(io_thread_globals->throttler_manager());

    url_request_context_ = context->AsWeakPtr();
    factory_->Initialize(context.Pass());

    factory_.reset();
  }

  return url_request_context_.get();
}

scoped_refptr<base::SingleThreadTaskRunner>
URLRequestContextGetter::GetNetworkTaskRunner() const {
  return content::BrowserThread::GetMessageLoopProxyForThread(
      content::BrowserThread::IO);
}

URLRequestContextGetter::~URLRequestContextGetter() {}

// static
URLRequestContextGetter* URLRequestContextGetter::CreateMain(
    BrowserContextIOData* context,
    content::ProtocolHandlerMap* protocol_handlers,
    content::ProtocolHandlerScopedVector protocol_interceptors) {
  return new URLRequestContextGetter(
      new MainURLRequestContextFactory(context,
                                       protocol_handlers,
                                       protocol_interceptors.Pass()));
}

} // namespace oxide
