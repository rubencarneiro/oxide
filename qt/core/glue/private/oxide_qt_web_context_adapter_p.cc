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

#include <QString>
#include <QUrl>

#include "base/logging.h"
#include "content/public/browser/browser_thread.h"
#include "net/base/net_errors.h"
#include "net/url_request/url_request.h"

#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_browser_context_delegate.h"
#include "qt/core/api/oxideqnetworkcallbackevents.h"
#include "qt/core/api/oxideqnetworkcallbackevents_p.h"
#include "qt/core/api/oxideqstoragepermissionrequest.h"
#include "qt/core/api/oxideqstoragepermissionrequest_p.h"

namespace oxide {
namespace qt {

class BrowserContextDelegate : public oxide::BrowserContextDelegate {
 public:
  BrowserContextDelegate(WebContextAdapterPrivate* ui_delegate,
                         WebContextAdapter::IOThreadDelegate* io_delegate) :
      ui_thread_delegate_(ui_delegate->AsWeakPtr()),
      io_thread_delegate_(io_delegate) {
  }

  virtual ~BrowserContextDelegate() {}

  WebContextAdapter::IOThreadDelegate* io_thread_delegate() const {
    return io_thread_delegate_.get();
  }

 private:
  virtual int OnBeforeURLRequest(net::URLRequest* request,
                                 const net::CompletionCallback& callback,
                                 GURL* new_url) {
    if (!io_thread_delegate_) {
      return net::OK;
    }

    bool cancelled = false;

    OxideQBeforeURLRequestEvent* event =
        new OxideQBeforeURLRequestEvent(
          QUrl(QString::fromStdString(request->url().spec())),
          QString::fromStdString(request->method()));

    OxideQBeforeURLRequestEventPrivate* eventp =
        OxideQBeforeURLRequestEventPrivate::get(event);
    eventp->request_cancelled = &cancelled;
    eventp->new_url = new_url;

    io_thread_delegate_->OnBeforeURLRequest(event);

    return cancelled ? net::ERR_ABORTED : net::OK;
  }

  virtual int OnBeforeSendHeaders(net::URLRequest* request,
                                  const net::CompletionCallback& callback,
                                  net::HttpRequestHeaders* headers) {
    if (!io_thread_delegate_) {
      return net::OK;
    }

    bool cancelled = false;

    OxideQBeforeSendHeadersEvent* event =
        new OxideQBeforeSendHeadersEvent(
          QUrl(QString::fromStdString(request->url().spec())),
          QString::fromStdString(request->method()));

    OxideQBeforeSendHeadersEventPrivate* eventp =
        OxideQBeforeSendHeadersEventPrivate::get(event);
    eventp->request_cancelled = &cancelled;
    eventp->headers = headers;

    io_thread_delegate_->OnBeforeSendHeaders(event);

    return cancelled ? net::ERR_ABORTED : net::OK;
  }

  virtual oxide::StoragePermission CanAccessStorage(
      const GURL& url,
      const GURL& first_party_url,
      bool write,
      oxide::StorageType type) {
    oxide::StoragePermission result = oxide::STORAGE_PERMISSION_UNDEFINED;

    if (!io_thread_delegate_) {
      return result;
    }

    OxideQStoragePermissionRequest* req =
        new OxideQStoragePermissionRequest(
          QUrl(QString::fromStdString(url.spec())),
          QUrl(QString::fromStdString(first_party_url.spec())),
          write,
          static_cast<OxideQStoragePermissionRequest::Type>(type));

    OxideQStoragePermissionRequestPrivate::get(req)->permission = &result;

    io_thread_delegate_->HandleStoragePermissionRequest(req);

    return result;
  }

  base::WeakPtr<WebContextAdapterPrivate> ui_thread_delegate_;
  scoped_ptr<WebContextAdapter::IOThreadDelegate> io_thread_delegate_;
};

WebContextAdapterPrivate::WebContextAdapterPrivate(
    WebContextAdapter* adapter,
    WebContextAdapter::IOThreadDelegate* io_delegate) :
    adapter(adapter),
    construct_props_(new ConstructProperties()),
    context_delegate_(new BrowserContextDelegate(this, io_delegate)) {}

WebContextAdapterPrivate::~WebContextAdapterPrivate() {}

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

WebContextAdapter::IOThreadDelegate*
WebContextAdapterPrivate::GetIOThreadDelegate() const {
  return context_delegate_->io_thread_delegate();
}

} // namespace qt
} // namespace oxide
