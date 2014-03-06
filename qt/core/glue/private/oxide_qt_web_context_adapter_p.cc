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

#include "../oxide_qt_web_context_adapter_p.h"
#include "../oxide_qt_web_context_adapter.h"

#include "base/logging.h"

#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_browser_context_delegate.h"

namespace oxide {
namespace qt {

class BrowserContextDelegate : public oxide::BrowserContextDelegate {
 public:
  BrowserContextDelegate() {}
  virtual ~BrowserContextDelegate() {}

  virtual int OnBeforeURLRequest(net::URLRequest* request,
                                 const net::CompletionCallback& callback,
                                 GURL* new_url) {
    return net::OK;
  }

  virtual int OnBeforeSendHeaders(net::URLRequest* request,
                                  const net::CompletionCallback& callback,
                                  net::HttpRequestHeaders* headers) {
    return net::OK;
  }

  virtual int OnHeadersReceived(
      net::URLRequest* request,
      const net::CompletionCallback& callback,
      const net::HttpResponseHeaders* original_response_headers,
      scoped_refptr<net::HttpResponseHeaders>* override_response_headers) {
    return net::OK;
  }
};

WebContextAdapterPrivate::WebContextAdapterPrivate() :
    construct_props_(new ConstructProperties()),
    context_delegate_(new BrowserContextDelegate()) {}

WebContextAdapterPrivate::~WebContextAdapterPrivate() {
  if (context()) {
    context()->SetDelegate(NULL);
  }
}

void WebContextAdapterPrivate::Init() {
  DCHECK(!context_);

  context_.reset(oxide::BrowserContext::Create(
      construct_props()->data_path,
      construct_props()->cache_path));

  if (!construct_props()->product.empty()) {
    context()->SetProduct(construct_props()->product);
  }
  if (!construct_props()->user_agent.empty()) {
    context()->SetUserAgent(construct_props()->user_agent);
  }
  if (!construct_props()->accept_langs.empty()) {
    context()->SetAcceptLangs(construct_props()->accept_langs);
  }
  context()->SetDelegate(context_delegate_);

  construct_props_.reset();
}

// static
WebContextAdapterPrivate* WebContextAdapterPrivate::get(
    WebContextAdapter* adapter) {
  return adapter->priv.data();
}

} // namespace qt
} // namespace oxide
