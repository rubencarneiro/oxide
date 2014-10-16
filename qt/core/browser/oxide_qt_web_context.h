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

#ifndef _OXIDE_QT_CORE_BROWSER_WEB_CONTEXT_H_
#define _OXIDE_QT_CORE_BROWSER_WEB_CONTEXT_H_

#include <QList>
#include <QSharedPointer>
#include <QtGlobal>
#include <QUrl>
#include <QWeakPointer>
#include <set>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/synchronization/lock.h"
#include "content/public/browser/cookie_store_factory.h"
#include "net/base/static_cookie_policy.h"
#include "net/cookies/canonical_cookie.h"

#include "qt/core/glue/oxide_qt_web_context_adapter.h"

#include "shared/browser/oxide_browser_context_delegate.h"

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
QT_END_NAMESPACE

namespace net {
class CookieStore;
}

namespace oxide {

class BrowserContext;

namespace qt {

struct ConstructProperties;
class SetCookiesContext;
class UserScriptAdapter;

class WebContext final : public oxide::BrowserContextDelegate {
 public:
  ~WebContext();

  static WebContext* FromAdapter(WebContextAdapter* adapter);
  static WebContext* FromBrowserContext(oxide::BrowserContext* context);

  oxide::BrowserContext* GetContext();

  QNetworkAccessManager* GetCustomNetworkAccessManager();

  bool IsInitialized() const;

  std::string GetProduct() const;
  void SetProduct(const std::string& product);

  std::string GetUserAgent() const;
  void SetUserAgent(const std::string& user_agent);

  base::FilePath GetDataPath() const;
  void SetDataPath(const base::FilePath& path);

  base::FilePath GetCachePath() const;
  void SetCachePath(const base::FilePath& path);

  std::string GetAcceptLangs() const;
  void SetAcceptLangs(const std::string& langs);

  net::StaticCookiePolicy::Type GetCookiePolicy() const;
  void SetCookiePolicy(net::StaticCookiePolicy::Type policy);

  content::CookieStoreConfig::SessionCookieMode GetSessionCookieMode() const;
  void SetSessionCookieMode(
      content::CookieStoreConfig::SessionCookieMode mode);

  bool GetPopupBlockerEnabled() const;
  void SetPopupBlockerEnabled(bool enabled);

  bool GetDevtoolsEnabled() const;
  void SetDevtoolsEnabled(bool enabled);
  int GetDevtoolsPort() const;
  void SetDevtoolsPort(int port);
  std::string GetDevtoolsBindIp() const;
  void SetDevtoolsBindIp(const std::string& ip);

  std::vector<std::string> GetHostMappingRules() const;
  void SetHostMappingRules(const std::vector<std::string>& rules);

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
    content::CookieStoreConfig::SessionCookieMode session_cookie_mode;
    bool popup_blocker_enabled;
    bool devtools_enabled;
    int devtools_port;
    std::string devtools_ip;
    std::vector<std::string> host_mapping_rules;
  };

  static WebContext* Create(WebContextAdapter* adapter);
  WebContext(WebContextAdapter* adapter);

  void Init(const QWeakPointer<WebContextAdapter::IODelegate>& io_delegate);
  void Destroy();

  WebContextAdapter* GetAdapter() const;
  QSharedPointer<WebContextAdapter::IODelegate> GetIODelegate() const;

  void UpdateUserScripts();

  int SetCookies(const QUrl& url,
                 const QList<QNetworkCookie>& cookies);
  void CookieSetCallback(const scoped_refptr<SetCookiesContext>& context,
                         const QNetworkCookie& cookie,
                         bool success);
  void DeliverCookiesSet(const scoped_refptr<SetCookiesContext>& ctxt);

  int GetCookies(const QUrl& url);
  int GetAllCookies();
  void GotCookiesCallback(int request_id,
                          const net::CookieList& cookies);

  int DeleteAllCookies();
  void DeletedCookiesCallback(int request_id, int num_deleted);

  void SetAllowedExtraURLSchemes(const std::set<std::string>& schemes);

  // oxide::BrowserContextDelegate
  int OnBeforeURLRequest(net::URLRequest* request,
                         const net::CompletionCallback& callback,
                         GURL* new_url) final;
  int OnBeforeSendHeaders(net::URLRequest* request,
                          const net::CompletionCallback& callback,
                          net::HttpRequestHeaders* headers) final;
  void OnBeforeRedirect(net::URLRequest* request,
                        const GURL& new_location) final;
  oxide::StoragePermission CanAccessStorage(
      const GURL& url,
      const GURL& first_party_url,
      bool write,
      oxide::StorageType type) final;
  bool GetUserAgentOverride(const GURL& url,
                            std::string* user_agent) final;
  bool IsCustomProtocolHandlerRegistered(
      const std::string& scheme) const final;
  oxide::URLRequestDelegatedJob* CreateCustomURLRequestJob(
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate) final;

  WebContextAdapter* adapter_;

  mutable base::Lock io_delegate_lock_;
  QWeakPointer<WebContextAdapter::IODelegate> io_delegate_;

  scoped_refptr<BrowserContext> context_;
  scoped_ptr<ConstructProperties> construct_props_;

  bool handling_cookie_request_;

  QList<UserScriptAdapter *> user_scripts_;

  mutable base::Lock url_schemes_lock_;
  std::set<std::string> allowed_extra_url_schemes_;

  DISALLOW_COPY_AND_ASSIGN(WebContext);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_PRIVATE_WEB_CONTEXT_ADAPTER_P_H_
