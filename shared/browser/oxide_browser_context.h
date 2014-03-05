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

namespace net {

class FtpNetworkLayer;
class HttpServerProperties;
class HttpUserAgentSettings;
class SSLConfigService;

}

namespace oxide {

class BrowserContextObserver;
class ResourceContext;
class URLRequestContext;
class URLRequestContextGetter;
class UserScriptMaster;

class BrowserContextIOData {
 public:
  virtual ~BrowserContextIOData();

  virtual base::FilePath GetPath() const = 0;
  virtual base::FilePath GetCachePath() const = 0;

  virtual std::string GetAcceptLangs() const = 0;
  virtual void SetAcceptLangs(const std::string& langs) = 0;

  virtual std::string GetProduct() const = 0;
  virtual void SetProduct(const std::string& product) = 0;

  virtual std::string GetUserAgent() const = 0;
  virtual void SetUserAgent(const std::string& user_agent) = 0;

  virtual bool IsOffTheRecord() const = 0;

  void Init(content::ProtocolHandlerMap& protocol_handlers);

  URLRequestContext* GetMainRequestContext();
  content::ResourceContext* GetResourceContext();

 protected:
  BrowserContextIOData();

  mutable base::Lock lock_;
  bool initialized_;

 private:
  scoped_refptr<net::SSLConfigService> ssl_config_service_;
  scoped_ptr<net::HttpUserAgentSettings> http_user_agent_settings_;
  scoped_ptr<net::FtpNetworkLayer> ftp_transaction_factory_;
  scoped_ptr<net::HttpServerProperties> http_server_properties_;
  scoped_ptr<net::NetworkDelegate> network_delegate_;

  scoped_ptr<URLRequestContext> main_request_context_;
  scoped_ptr<ResourceContext> resource_context_;
};

class BrowserContext : public content::BrowserContext {
 public:
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
  static BrowserContext* Create(const base::FilePath& path,
                                const base::FilePath& cache_path);

  static std::vector<BrowserContext *>& GetAllContexts();

  // Aborts if there are any live contexts
  static void AssertNoContextsExist();

  net::URLRequestContextGetter* CreateRequestContext(
      content::ProtocolHandlerMap* protocol_handlers);

  virtual BrowserContext* GetOffTheRecordContext() = 0;
  virtual BrowserContext* GetOriginalContext() = 0;

  bool IsOffTheRecord() const FINAL;

  bool IsSameContext(BrowserContext* other) const;

  base::FilePath GetPath() const FINAL;
  base::FilePath GetCachePath() const;

  std::string GetAcceptLangs() const;
  void SetAcceptLangs(const std::string& langs);

  std::string GetProduct() const;
  void SetProduct(const std::string& product);

  std::string GetUserAgent() const;
  void SetUserAgent(const std::string& user_agent);

  BrowserContextIOData* io_data() const { return io_data_handle_.io_data(); }

  virtual UserScriptMaster& UserScriptManager() = 0;

  virtual net::URLRequestContextGetter* GetRequestContext() OVERRIDE;
  virtual net::URLRequestContextGetter* GetRequestContextForRenderProcess(
      int renderer_child_id) OVERRIDE;

  virtual net::URLRequestContextGetter* GetMediaRequestContext() OVERRIDE;
  virtual net::URLRequestContextGetter* GetMediaRequestContextForRenderProcess(
      int renderer_child_id) OVERRIDE;

  virtual net::URLRequestContextGetter*
      GetMediaRequestContextForStoragePartition(
          const base::FilePath& partition_path,
          bool in_memory) OVERRIDE;

  virtual void RequestMidiSysExPermission(
      int render_process_id,
      int render_view_id,
      int bridge_id,
      const GURL& requesting_frame,
      const MidiSysExPermissionCallback& callback) OVERRIDE;

  virtual void CancelMidiSysExPermissionRequest(
      int render_process_id,
      int render_view_id,
      int bridge_id,
      const GURL& requesting_frame) OVERRIDE;

  virtual void RequestProtectedMediaIdentifierPermission(
      int render_process_id,
      int render_view_id,
      int bridge_id,
      int group_id,
      const GURL& requesting_frame,
      const ProtectedMediaIdentifierPermissionCallback& callback) OVERRIDE;

  virtual void CancelProtectedMediaIdentifierPermissionRequests(
      int group_id) OVERRIDE;

  virtual content::ResourceContext* GetResourceContext() OVERRIDE;

  virtual content::DownloadManagerDelegate*
      GetDownloadManagerDelegate() OVERRIDE;

  virtual content::GeolocationPermissionContext*
      GetGeolocationPermissionContext() OVERRIDE;

  virtual quota::SpecialStoragePolicy* GetSpecialStoragePolicy() OVERRIDE;

 protected:
  BrowserContext(BrowserContextIOData* io_data);

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

  void AddObserver(BrowserContextObserver* observer);
  void RemoveObserver(BrowserContextObserver* observer);

  IODataHandle io_data_handle_;
  scoped_refptr<URLRequestContextGetter> main_request_context_getter_;
  ObserverList<BrowserContextObserver> observers_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(BrowserContext);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_H_
