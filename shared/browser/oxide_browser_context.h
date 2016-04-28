// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_H_
#define _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_H_

#include <string>

#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/observer_list.h"
#include "base/synchronization/lock.h"
#include "base/threading/non_thread_safe.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/cookie_store_factory.h"
#include "net/base/static_cookie_policy.h"

#include "shared/common/oxide_shared_export.h"

namespace content {
class ResourceContext;
}

namespace net {

class CookieMonster;
class FtpNetworkLayer;
class HostMappingRules;
class HttpNetworkSession;
class HttpServerProperties;
class HttpUserAgentSettings;
class SSLConfigService;
class TransportSecurityPersister;
class TransportSecurityState;

}

namespace oxide {

class BrowserContext;
typedef uintptr_t BrowserContextID;

class BrowserContextDelegate;
class BrowserContextImpl;
class BrowserContextObserver;
class BrowserContextSharedData;
class BrowserContextSharedIOData;
class CookieStoreUIProxy;
class GeolocationPermissionContext;
class PermissionManager;
class ResourceContext;
class SSLHostStateDelegate;
class TemporarySavedPermissionContext;
class URLRequestContext;
class URLRequestContextGetter;
class UserAgentSettingsIOData;

class BrowserContextIOData {
 public:
  virtual ~BrowserContextIOData();

  static BrowserContextIOData* FromResourceContext(
      content::ResourceContext* context);

  scoped_refptr<BrowserContextDelegate> GetDelegate();

  net::StaticCookiePolicy::Type GetCookiePolicy() const;
  virtual content::CookieStoreConfig::SessionCookieMode GetSessionCookieMode() const = 0;

  bool IsPopupBlockerEnabled() const;

  base::FilePath GetPath() const;
  base::FilePath GetCachePath() const;
  int GetMaxCacheSizeHint() const;

  bool GetDoNotTrack() const;

  virtual bool IsOffTheRecord() const = 0;

  URLRequestContext* CreateMainRequestContext(
      content::ProtocolHandlerMap& protocol_handlers,
      content::URLRequestInterceptorScopedVector request_interceptors);

  content::ResourceContext* GetResourceContext();

  bool CanAccessCookies(const GURL& url,
                        const GURL& first_party_url,
                        bool write);

  // XXX: This will be going away
  // (see the comment in oxide_temporary_saved_permission_context.h)
  TemporarySavedPermissionContext* GetTemporarySavedPermissionContext() const;

  UserAgentSettingsIOData* GetUserAgentSettings() const;

  net::CookieStore* GetCookieStore() const;

 protected:
  friend class BrowserContextImpl; // For GetSharedData()

  BrowserContextIOData();

  virtual BrowserContextSharedIOData& GetSharedData() = 0;
  virtual const BrowserContextSharedIOData& GetSharedData() const = 0;

 private:
  friend class BrowserContext; // For Init(), GetSharedData() and various members

  void Init();

  scoped_refptr<net::SSLConfigService> ssl_config_service_;
  std::unique_ptr<net::HttpUserAgentSettings> http_user_agent_settings_;
  std::unique_ptr<net::FtpNetworkLayer> ftp_transaction_factory_;
  std::unique_ptr<net::HttpServerProperties> http_server_properties_;
  std::unique_ptr<net::NetworkDelegate> network_delegate_;

  std::unique_ptr<net::TransportSecurityState> transport_security_state_;
  std::unique_ptr<net::TransportSecurityPersister> transport_security_persister_;

  std::unique_ptr<net::HttpNetworkSession> http_network_session_;

  std::unique_ptr<URLRequestContext> main_request_context_;
  std::unique_ptr<ResourceContext> resource_context_;
  std::unique_ptr<net::CookieStore> cookie_store_;

  std::unique_ptr<net::HostMappingRules> host_mapping_rules_;

  std::unique_ptr<TemporarySavedPermissionContext>
      temporary_saved_permission_context_;
};

class BrowserContext;

struct OXIDE_SHARED_EXPORT BrowserContextTraits {
  static void Destruct(const BrowserContext* x);
};

// This class holds the context needed for a browsing session. It lives on
// and must only be accessed on the UI thread - note that it uses a thread-safe
// refcount only so that we can override the delete behaviour
class OXIDE_SHARED_EXPORT BrowserContext
    : public content::BrowserContext,
      public base::RefCountedThreadSafe<BrowserContext, BrowserContextTraits>,
      public base::NonThreadSafe {
 public:

  struct Params {
    Params(const base::FilePath& path,
           const base::FilePath& cache_path,
           int max_cache_size_hint,
           content::CookieStoreConfig::SessionCookieMode session_cookie_mode)
           : path(path),
             cache_path(cache_path),
             max_cache_size_hint(max_cache_size_hint),
             session_cookie_mode(session_cookie_mode) {}

    base::FilePath path;
    base::FilePath cache_path;
    int max_cache_size_hint;
    content::CookieStoreConfig::SessionCookieMode session_cookie_mode;
    std::vector<std::string> host_mapping_rules;
  };

  static BrowserContext* FromContent(
      content::BrowserContext* context) {
    return static_cast<BrowserContext *>(context);
  }

  static scoped_refptr<BrowserContext> Create(const Params& params);

  typedef base::Callback<void(BrowserContext*)> BrowserContextCallback;
  static void ForEach(const BrowserContextCallback& callback);

  // Aborts if there are any live contexts
  static void AssertNoContextsExist();

  BrowserContextID GetID() const;

  BrowserContextDelegate* GetDelegate() const;
  void SetDelegate(BrowserContextDelegate* delegate);

  virtual scoped_refptr<BrowserContext> GetOffTheRecordContext() = 0;
  virtual BrowserContext* GetOriginalContext() const = 0;
  virtual bool HasOffTheRecordContext() const = 0;

  bool IsOffTheRecord() const override; // from content::BrowserContext

  bool IsSameContext(BrowserContext* other) const;

  base::FilePath GetPath() const override; // from content::BrowserContext
  base::FilePath GetCachePath() const;
  int GetMaxCacheSizeHint() const;

  net::StaticCookiePolicy::Type GetCookiePolicy() const;
  void SetCookiePolicy(net::StaticCookiePolicy::Type policy);

  content::CookieStoreConfig::SessionCookieMode GetSessionCookieMode() const;

  bool IsPopupBlockerEnabled() const;
  void SetIsPopupBlockerEnabled(bool enabled);

  const std::vector<std::string>& GetHostMappingRules() const;

  bool GetDoNotTrack() const;
  void SetDoNotTrack(bool dnt);

  // from content::BrowserContext
  content::ResourceContext* GetResourceContext() override;

  net::CookieStore* GetCookieStore() const;

  // XXX: This will be going away
  // (see the comment in oxide_temporary_saved_permission_context.h)
  TemporarySavedPermissionContext* GetTemporarySavedPermissionContext() const;

  BrowserContextIOData* GetIOData() const;

 protected:
  friend class BrowserContextDestroyer; // for destructor

  BrowserContext(BrowserContextIOData* io_data);
  virtual ~BrowserContext();

  BrowserContextIOData* io_data() const { return io_data_; }

 private:
  friend class BrowserContextObserver; // for {Add,Remove}Observer

  // content::BrowserContext implementation
  std::unique_ptr<content::ZoomLevelDelegate> CreateZoomLevelDelegate(
      const base::FilePath& partition_path) override;
  content::DownloadManagerDelegate* GetDownloadManagerDelegate() override;
  content::BrowserPluginGuestManager* GetGuestManager() override;
  storage::SpecialStoragePolicy* GetSpecialStoragePolicy() override;
  content::PushMessagingService* GetPushMessagingService() override;
  content::SSLHostStateDelegate* GetSSLHostStateDelegate() override;
  content::PermissionManager* GetPermissionManager() override;
  content::BackgroundSyncController* GetBackgroundSyncController() override;
  net::URLRequestContextGetter* CreateRequestContext(
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector request_interceptors) override;
  net::URLRequestContextGetter* CreateRequestContextForStoragePartition(
      const base::FilePath& partition_path,
      bool in_memory,
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector request_interceptors) override;
  net::URLRequestContextGetter* CreateMediaRequestContext() override;
  net::URLRequestContextGetter* CreateMediaRequestContextForStoragePartition(
      const base::FilePath& partition_path,
      bool in_memory) override;

  void AddObserver(BrowserContextObserver* observer);
  void RemoveObserver(BrowserContextObserver* observer);

  BrowserContextIOData* io_data_;
  scoped_refptr<URLRequestContextGetter> main_request_context_getter_;
  base::ObserverList<BrowserContextObserver> observers_;

  std::unique_ptr<SSLHostStateDelegate> ssl_host_state_delegate_;
  std::unique_ptr<PermissionManager> permission_manager_;

  std::unique_ptr<CookieStoreUIProxy> cookie_store_;

  DISALLOW_COPY_AND_ASSIGN(BrowserContext);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_H_
