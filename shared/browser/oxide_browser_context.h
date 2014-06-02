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

#ifndef _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_H_
#define _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "base/synchronization/lock.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/cookie_store_factory.h"
#include "net/base/static_cookie_policy.h"

namespace content {
class ResourceContext;
}

namespace net {

class FtpNetworkLayer;
class HttpServerProperties;
class HttpUserAgentSettings;
class SSLConfigService;
class TransportSecurityPersister;
class TransportSecurityState;

}

namespace oxide {

class BrowserContextDelegate;
class BrowserContextObserver;
class GeolocationPermissionContext;
class ResourceContext;
class URLRequestContext;
class URLRequestContextGetter;
class UserScriptMaster;

class BrowserContextIOData {
 public:
  virtual ~BrowserContextIOData();

  static BrowserContextIOData* FromResourceContext(
      content::ResourceContext* context);

  scoped_refptr<BrowserContextDelegate> GetDelegate();

  virtual net::StaticCookiePolicy::Type GetCookiePolicy() const = 0;
  virtual content::CookieStoreConfig::SessionCookieMode GetSessionCookieMode() const = 0;

  virtual bool IsPopupBlockerEnabled() const = 0;

  virtual base::FilePath GetPath() const = 0;
  virtual base::FilePath GetCachePath() const = 0;

  virtual std::string GetAcceptLangs() const = 0;

  virtual std::string GetUserAgent() const = 0;

  virtual bool IsOffTheRecord() const = 0;

  URLRequestContext* CreateMainRequestContext(
      content::ProtocolHandlerMap& protocol_handlers,
      content::ProtocolHandlerScopedVector protocol_interceptors);

  content::ResourceContext* GetResourceContext();

  bool CanAccessCookies(const GURL& url,
                        const GURL& first_party_url,
                        bool write);

 protected:
  BrowserContextIOData();

 private:
  friend class BrowserContext;

  void SetDelegate(BrowserContextDelegate* delegate);

  base::Lock delegate_lock_;
  scoped_refptr<BrowserContextDelegate> delegate_;

  scoped_refptr<net::SSLConfigService> ssl_config_service_;
  scoped_ptr<net::HttpUserAgentSettings> http_user_agent_settings_;
  scoped_ptr<net::FtpNetworkLayer> ftp_transaction_factory_;
  scoped_ptr<net::HttpServerProperties> http_server_properties_;
  scoped_ptr<net::NetworkDelegate> network_delegate_;

  scoped_ptr<net::TransportSecurityState> transport_security_state_;
  scoped_ptr<net::TransportSecurityPersister> transport_security_persister_;

  scoped_ptr<URLRequestContext> main_request_context_;
  scoped_ptr<ResourceContext> resource_context_;
};

class BrowserContext : public content::BrowserContext,
                       public base::RefCounted<BrowserContext> {
 public:

  struct Params {
    Params(const base::FilePath& path,
           const base::FilePath& cache_path,
           const content::CookieStoreConfig::SessionCookieMode session_cookie_mode) :
        path(path), cache_path(cache_path), session_cookie_mode(session_cookie_mode) {}

    base::FilePath path;
    base::FilePath cache_path;
    content::CookieStoreConfig::SessionCookieMode session_cookie_mode;
  };

  virtual ~BrowserContext();

  static BrowserContext* FromContent(
      content::BrowserContext* context) {
    return static_cast<BrowserContext *>(context);
  }

  // Create a new browser context. The caller owns this context, and
  // is responsible for destroying it when it is finished with it.
  // The caller must ensure that it outlives any other consumers (ie,
  // WebView's), and must ensure that it is destroyed before all
  // references to the BrowserProcessMain have been released
  static BrowserContext* Create(const Params& params);

  // Aborts if there are any live contexts
  static void AssertNoContextsExist();

  net::URLRequestContextGetter* CreateRequestContext(
      content::ProtocolHandlerMap* protocol_handlers,
      content::ProtocolHandlerScopedVector protocol_interceptors);

  BrowserContextDelegate* GetDelegate() const;
  void SetDelegate(BrowserContextDelegate* delegate);

  virtual BrowserContext* GetOffTheRecordContext() = 0;
  virtual BrowserContext* GetOriginalContext() = 0;

  bool IsOffTheRecord() const FINAL;

  bool IsSameContext(BrowserContext* other) const;

  base::FilePath GetPath() const FINAL;
  base::FilePath GetCachePath() const;

  std::string GetAcceptLangs() const;
  virtual void SetAcceptLangs(const std::string& langs) = 0;

  virtual std::string GetProduct() const = 0;
  virtual void SetProduct(const std::string& product) = 0;

  virtual std::string GetUserAgent() const;
  virtual void SetUserAgent(const std::string& user_agent) = 0;

  net::StaticCookiePolicy::Type GetCookiePolicy() const;
  virtual void SetCookiePolicy(net::StaticCookiePolicy::Type policy) = 0;

  content::CookieStoreConfig::SessionCookieMode GetSessionCookieMode() const;

  bool IsPopupBlockerEnabled() const;
  virtual void SetIsPopupBlockerEnabled(bool enabled) = 0;

  BrowserContextIOData* io_data() const { return io_data_handle_.io_data(); }

  virtual UserScriptMaster& UserScriptManager() = 0;

  content::ResourceContext* GetResourceContext() FINAL;

 protected:
  BrowserContext(BrowserContextIOData* io_data);

  void OnPopupBlockerEnabledChanged();

 private:
  friend class BrowserContextObserver;

  class IODataHandle {
   public:
    IODataHandle(BrowserContextIOData* data) : io_data_(data) {}
    ~IODataHandle();

    BrowserContextIOData* io_data() const { return io_data_; }

   private:
    BrowserContextIOData* io_data_;
  };

  net::URLRequestContextGetter* GetRequestContext() FINAL;
  net::URLRequestContextGetter* GetRequestContextForRenderProcess(
      int renderer_child_id) FINAL;

  net::URLRequestContextGetter* GetMediaRequestContext() FINAL;
  net::URLRequestContextGetter* GetMediaRequestContextForRenderProcess(
      int renderer_child_id) FINAL;

  net::URLRequestContextGetter*
      GetMediaRequestContextForStoragePartition(
          const base::FilePath& partition_path,
          bool in_memory) FINAL;

  void RequestMidiSysExPermission(
      int render_process_id,
      int render_view_id,
      int bridge_id,
      const GURL& requesting_frame,
      bool user_gesture,
      const MidiSysExPermissionCallback& callback) FINAL;

  void CancelMidiSysExPermissionRequest(
      int render_process_id,
      int render_view_id,
      int bridge_id,
      const GURL& requesting_frame) FINAL;

  void RequestProtectedMediaIdentifierPermission(
      int render_process_id,
      int render_view_id,
      int bridge_id,
      int group_id,
      const GURL& requesting_frame,
      const ProtectedMediaIdentifierPermissionCallback& callback) FINAL;

  void CancelProtectedMediaIdentifierPermissionRequests(
      int group_id) FINAL;

  content::DownloadManagerDelegate* GetDownloadManagerDelegate() FINAL;

  content::GeolocationPermissionContext*
      GetGeolocationPermissionContext() FINAL;

  content::BrowserPluginGuestManager* GetGuestManager() FINAL;

  quota::SpecialStoragePolicy* GetSpecialStoragePolicy() FINAL;

  void AddObserver(BrowserContextObserver* observer);
  void RemoveObserver(BrowserContextObserver* observer);

  IODataHandle io_data_handle_;
  scoped_refptr<URLRequestContextGetter> main_request_context_getter_;
  ObserverList<BrowserContextObserver> observers_;

  scoped_refptr<GeolocationPermissionContext> geolocation_permission_context_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(BrowserContext);
};

class ScopedBrowserContext FINAL {
 public:
  ScopedBrowserContext() :
      context_(NULL),
      ref_context_(NULL) {}
  ScopedBrowserContext(BrowserContext* context) :
      context_(context),
      ref_context_(context ? context->GetOriginalContext() : NULL) {
    if (ref_context_) {
      ref_context_->AddRef();
    }
  }
  ScopedBrowserContext(const ScopedBrowserContext& other) :
      context_(other.context_),
      ref_context_(other.ref_context_) {
    if (ref_context_) {
      ref_context_->AddRef();
    }
  }

  ~ScopedBrowserContext() {
    if (ref_context_) {
      ref_context_->Release();
    }
  }

  BrowserContext* get() const { return context_; }
  operator BrowserContext*() const { return context_; }
  BrowserContext* operator->() const {
    assert(context_);
    return context_;
  }

  ScopedBrowserContext& operator=(BrowserContext* context) {
    BrowserContext* ref_context = context ? context->GetOriginalContext() : NULL;
    if (ref_context) {
      ref_context->AddRef();
    }
    BrowserContext* old = ref_context_;
    context_ = context;
    ref_context_ = ref_context;
    if (old) {
      old->Release();
    }
    return *this;
  }

  ScopedBrowserContext& operator=(const ScopedBrowserContext& other) {
    *this = other.get();
    return *this;
  }

 private:
  BrowserContext* context_;
  BrowserContext* ref_context_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_H_
