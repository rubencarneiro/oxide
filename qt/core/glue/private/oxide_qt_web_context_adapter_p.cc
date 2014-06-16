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

#include "qt/core/api/oxideqnetworkcallbackevents.h"
#include "qt/core/api/oxideqnetworkcallbackevents_p.h"
#include "qt/core/api/oxideqstoragepermissionrequest.h"
#include "qt/core/api/oxideqstoragepermissionrequest_p.h"
#include "qt/core/browser/oxide_qt_render_widget_host_view_factory.h"
#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_browser_context_delegate.h"
#include "shared/browser/oxide_user_script_master.h"

#include "../oxide_qt_render_widget_host_view_delegate_factory.h"
#include "../oxide_qt_user_script_adapter.h"
#include "../oxide_qt_user_script_adapter_p.h"

namespace oxide {
namespace qt {

WebContextAdapterPrivate::ConstructProperties::ConstructProperties() :
    cookie_policy(net::StaticCookiePolicy::ALLOW_ALL_COOKIES),
    session_cookie_mode(content::CookieStoreConfig::EPHEMERAL_SESSION_COOKIES),
    popup_blocker_enabled(true),
    devtools_enabled(false),
    devtools_port(0) {}

// static
WebContextAdapterPrivate* WebContextAdapterPrivate::Create(
    WebContextAdapter* adapter,
    WebContextAdapter::IOThreadDelegate* io_delegate,
    RenderWidgetHostViewDelegateFactory* view_factory) {
  return new WebContextAdapterPrivate(adapter, io_delegate, view_factory);
}

WebContextAdapterPrivate::WebContextAdapterPrivate(
    WebContextAdapter* adapter,
    WebContextAdapter::IOThreadDelegate* io_delegate,
    RenderWidgetHostViewDelegateFactory* view_factory) :
    adapter_(adapter),
    io_thread_delegate_(io_delegate),
    view_factory_(view_factory),
    construct_props_(new ConstructProperties()) {}

void WebContextAdapterPrivate::Destroy() {
  if (context_) {
    context_->SetDelegate(NULL);
  }
  adapter_ = NULL;
}

void WebContextAdapterPrivate::UpdateUserScripts() {
  if (!context_) {
    return;
  }

  std::vector<oxide::UserScript *> scripts;

  for (int i = 0; i < user_scripts_.size(); ++i) {
    UserScriptAdapterPrivate* script =
        UserScriptAdapterPrivate::get(user_scripts_.at(i));
    if (script->state == UserScriptAdapterPrivate::Loading ||
        script->state == UserScriptAdapterPrivate::Constructing) {
      return;
    } else if (script->state == UserScriptAdapterPrivate::Loaded) {
      scripts.push_back(&script->user_script);
    }
  }

  context_->UserScriptManager().SerializeUserScriptsAndSendUpdates(scripts);
}

int WebContextAdapterPrivate::OnBeforeURLRequest(
    net::URLRequest* request,
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

int WebContextAdapterPrivate::OnBeforeSendHeaders(
    net::URLRequest* request,
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

oxide::StoragePermission WebContextAdapterPrivate::CanAccessStorage(
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

bool WebContextAdapterPrivate::GetUserAgentOverride(const GURL& url,
                                                    std::string* user_agent) {
  if (!io_thread_delegate_) {
    return false;
  }

  QString new_user_agent;
  bool overridden = io_thread_delegate_->GetUserAgentOverride(
      QUrl(QString::fromStdString(url.spec())), &new_user_agent);

  *user_agent = new_user_agent.toStdString();
  return overridden;
}

WebContextAdapterPrivate::~WebContextAdapterPrivate() {}

// static
WebContextAdapterPrivate* WebContextAdapterPrivate::get(
    WebContextAdapter* adapter) {
  return adapter->priv;
}

// static
WebContextAdapterPrivate* WebContextAdapterPrivate::FromBrowserContext(
    oxide::BrowserContext* context) {
  return static_cast<WebContextAdapterPrivate *>(context->GetDelegate());
}

oxide::BrowserContext* WebContextAdapterPrivate::GetContext() {
  if (context_) {
    return context_;
  }

  DCHECK(construct_props_);

  oxide::BrowserContext::Params params(
      construct_props_->data_path,
      construct_props_->cache_path,
      construct_props_->session_cookie_mode,
      construct_props_->devtools_enabled,
      construct_props_->devtools_port);
  context_ = oxide::BrowserContext::Create(params);

  if (!construct_props_->product.empty()) {
    context_->SetProduct(construct_props_->product);
  }
  if (!construct_props_->user_agent.empty()) {
    context_->SetUserAgent(construct_props_->user_agent);
  }
  if (!construct_props_->accept_langs.empty()) {
    context_->SetAcceptLangs(construct_props_->accept_langs);
  }
  context_->SetCookiePolicy(construct_props_->cookie_policy);
  context_->SetIsPopupBlockerEnabled(construct_props_->popup_blocker_enabled);

  context_->SetDelegate(this);

  construct_props_.reset();

  // BrowserContext takes ownership of this
  new RenderWidgetHostViewFactory(context_.get(), view_factory_.release());

  UpdateUserScripts();

  return context_;
}

} // namespace qt
} // namespace oxide
