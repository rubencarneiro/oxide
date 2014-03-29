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

#ifndef _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_ADAPTER_P_H_
#define _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_ADAPTER_P_H_

#include <QList>
#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "net/base/static_cookie_policy.h"

#include "qt/core/glue/oxide_qt_web_context_adapter.h"

#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_browser_context_delegate.h"

namespace oxide {

class BrowserContext;

namespace qt {

class BrowserContextDelegate;
struct ConstructProperties;
class RenderWidgetHostViewDelegateFactory;

class WebContextAdapterPrivate FINAL : public oxide::BrowserContextDelegate {
 public:
  ~WebContextAdapterPrivate();

  static WebContextAdapterPrivate* get(WebContextAdapter* adapter);
  static WebContextAdapterPrivate* FromBrowserContext(
      oxide::BrowserContext* context);

  WebContextAdapter* adapter() const { return adapter_; }
  oxide::BrowserContext* context() const { return context_; }

 private:
  friend class WebContextAdapter;

  struct ConstructProperties {
    ConstructProperties();

    std::string product;
    std::string user_agent;
    base::FilePath data_path;
    base::FilePath cache_path;
    std::string accept_langs;
    net::StaticCookiePolicy::Type cookie_policy;
    bool popup_blocker_enabled;
  };

  static WebContextAdapterPrivate* Create(
      WebContextAdapter* adapter,
      WebContextAdapter::IOThreadDelegate* io_delegate,
      RenderWidgetHostViewDelegateFactory* view_factory);
  WebContextAdapterPrivate(WebContextAdapter* adapter,
                           WebContextAdapter::IOThreadDelegate* io_delegate,
                           RenderWidgetHostViewDelegateFactory* view_factory);

  void Init();
  void Destroy();

  // oxide::BrowserContextDelegate
  int OnBeforeURLRequest(net::URLRequest* request,
                         const net::CompletionCallback& callback,
                         GURL* new_url) FINAL;
  int OnBeforeSendHeaders(net::URLRequest* request,
                          const net::CompletionCallback& callback,
                          net::HttpRequestHeaders* headers) FINAL;
  oxide::StoragePermission CanAccessStorage(
      const GURL& url,
      const GURL& first_party_url,
      bool write,
      oxide::StorageType type) FINAL;
  bool GetUserAgentOverride(const GURL& url,
                            std::string* user_agent) FINAL;

  WebContextAdapter* adapter_;
  scoped_ptr<WebContextAdapter::IOThreadDelegate> io_thread_delegate_;
  scoped_ptr<RenderWidgetHostViewDelegateFactory> view_factory_;

  ScopedBrowserContext context_;
  scoped_ptr<ConstructProperties> construct_props_;

  DISALLOW_COPY_AND_ASSIGN(WebContextAdapterPrivate);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_PRIVATE_WEB_CONTEXT_ADAPTER_P_H_
