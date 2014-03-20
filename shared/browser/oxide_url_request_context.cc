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

namespace oxide {

class URLRequestContextFactory {
 public:
  URLRequestContextFactory() {}
  virtual ~URLRequestContextFactory() {}

  virtual URLRequestContext* Initialize() = 0;
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

  URLRequestContext* Initialize() FINAL {
    context_->Init(protocol_handlers_, protocol_interceptors_.Pass());
    return context_->GetMainRequestContext();
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
    url_request_context_ = factory_->Initialize()->AsWeakPtr();
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
