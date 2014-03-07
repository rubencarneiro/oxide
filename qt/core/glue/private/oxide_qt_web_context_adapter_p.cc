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

#include <QSharedPointer>
#include <QString>
#include <QUrl>

#include "base/logging.h"
#include "net/base/net_errors.h"
#include "net/url_request/url_request.h"

#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_browser_context_delegate.h"
#include "qt/core/api/oxideqnetworkcallbackevents.h"
#include "qt/core/api/oxideqnetworkcallbackevents_p.h"

namespace oxide {
namespace qt {

class BrowserContextDelegate : public oxide::BrowserContextDelegate {
 public:
  BrowserContextDelegate(WebContextAdapter* adapter,
                         WebContextAdapter::IOThreadDelegate* io_delegate) :
      ui_thread_delegate_(adapter),
      io_thread_delegate_(io_delegate) {}
  virtual ~BrowserContextDelegate() {}

  virtual int OnBeforeURLRequest(net::URLRequest* request,
                                 const net::CompletionCallback& callback,
                                 GURL* new_url) {
    if (!io_thread_delegate_) {
      return net::OK;
    }

    QSharedPointer<OxideQBeforeURLRequestEvent> event(
        new OxideQBeforeURLRequestEvent());
    OxideQBeforeURLRequestEventPrivate::get(event.data())->new_url = new_url;
    OxideQBeforeURLRequestEventPrivate::get(event.data())->current_url =
        QUrl(QString::fromStdString(request->url().spec()));

    io_thread_delegate_->OnBeforeURLRequest(event);

    return event->requestCancelled() ? net::ERR_ABORTED : net::OK;
  }

  virtual int OnBeforeSendHeaders(net::URLRequest* request,
                                  const net::CompletionCallback& callback,
                                  net::HttpRequestHeaders* headers) {
    if (!io_thread_delegate_) {
      return net::OK;
    }

    QSharedPointer<OxideQBeforeSendHeadersEvent> event(
        new OxideQBeforeSendHeadersEvent());
    OxideQBeforeSendHeadersEventPrivate::get(event.data())->headers = headers;

    io_thread_delegate_->OnBeforeSendHeaders(event);

    return event->requestCancelled() ? net::ERR_ABORTED : net::OK;
  }

 private:
  WebContextAdapter* ui_thread_delegate_;
  scoped_ptr<WebContextAdapter::IOThreadDelegate> io_thread_delegate_;
};

WebContextAdapterPrivate::WebContextAdapterPrivate(
    WebContextAdapter* adapter,
    WebContextAdapter::IOThreadDelegate* io_delegate) :
    construct_props_(new ConstructProperties()),
    context_delegate_(new BrowserContextDelegate(adapter, io_delegate)) {}

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
