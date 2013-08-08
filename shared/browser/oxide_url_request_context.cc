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

namespace {

class DefaultURLRequestContextGetter FINAL : public URLRequestContextGetter {
 public:
  DefaultURLRequestContextGetter(
      content::ProtocolHandlerMap* protocol_handlers,
      BrowserContextIOData* context) :
      URLRequestContextGetter(),
      context_(context) {
    std::swap(protocol_handlers_, *protocol_handlers);
  }

  net::URLRequestContext* GetURLRequestContext() OVERRIDE {
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    if (!url_request_context_) {
      context_->Init(protocol_handlers_);
      url_request_context_ = context_->GetMainRequestContext()->GetWeakPtr();
      context_ = NULL;
    }

    return url_request_context_.get();
  }

 private:
  content::ProtocolHandlerMap protocol_handlers_;
  // XXX: Can we outlive the context IO data?
  BrowserContextIOData* context_;

  DISALLOW_COPY_AND_ASSIGN(DefaultURLRequestContextGetter);
};

} // namespace

URLRequestContextGetter::URLRequestContextGetter() {}

URLRequestContextGetter::~URLRequestContextGetter() {}

// static
URLRequestContextGetter* URLRequestContextGetter::CreateMain(
    content::ProtocolHandlerMap* protocol_handlers,
    BrowserContextIOData* context) {
  return new DefaultURLRequestContextGetter(protocol_handlers,
                                            context);
}

scoped_refptr<base::SingleThreadTaskRunner>
URLRequestContextGetter::GetNetworkTaskRunner() const {
  return content::BrowserThread::GetMessageLoopProxyForThread(
      content::BrowserThread::IO);
}

} // namespace oxide
