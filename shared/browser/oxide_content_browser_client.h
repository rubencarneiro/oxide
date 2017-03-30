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

#ifndef _OXIDE_SHARED_BROWSER_CONTENT_BROWSER_CLIENT_H_
#define _OXIDE_SHARED_BROWSER_CONTENT_BROWSER_CLIENT_H_

#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/content_browser_client.h"

namespace content {
class QuotaPermissionContext;
class RenderViewHost;
class ResourceDispatcherHostDelegate;
}

namespace oxide {

class BrowserPlatformIntegration;
class ContentMainDelegate;
class ResourceDispatcherHostDelegate;

class ContentBrowserClient final : public content::ContentBrowserClient {
 public:
  ContentBrowserClient(const std::string& application_locale,
                       BrowserPlatformIntegration* integration);
  ~ContentBrowserClient();

  // XXX(chrisccoulson): Try not to add anything here

 private:
  // content::ContentBrowserClient implementation
  content::BrowserMainParts* CreateBrowserMainParts(
      const content::MainFunctionParams& parameters) override;
  void RenderProcessWillLaunch(content::RenderProcessHost* host) override;
  void SiteInstanceGotProcess(content::SiteInstance* site_instance) override;
  void AppendExtraCommandLineSwitches(base::CommandLine* command_line,
                                      int child_process_id) override;
  std::string GetApplicationLocale() override;
  std::string GetAcceptLangs(
      content::BrowserContext* browser_context) override;
  bool AllowGetCookie(const GURL& url,
                      const GURL& first_party,
                      const net::CookieList& cookie_list,
                      content::ResourceContext* context,
                      int render_process_id,
                      int render_frame_id) override;
  bool AllowSetCookie(const GURL& url,
                      const GURL& first_party,
                      const std::string& cookie_line,
                      content::ResourceContext* context,
                      int render_process_id,
                      int render_frame_id,
                      const net::CookieOptions& options) override;
  content::QuotaPermissionContext* CreateQuotaPermissionContext() override;
  void AllowCertificateError(
      content::WebContents* contents,
      bool is_main_frame,
      int cert_error,
      const net::SSLInfo& ssl_info,
      const GURL& request_url,
      content::ResourceType resource_type,
      bool overridable,
      bool strict_enforcement,
      bool expired_previous_decision,
      const base::Callback<void(content::CertificateRequestResultType)>& callback) override;
  content::MediaObserver* GetMediaObserver() override;
  content::PlatformNotificationService*
      GetPlatformNotificationService() override;
  bool CanCreateWindow(int opener_render_process_id,
                       int opener_render_frame_id,
                       const GURL& opener_url,
                       const GURL& opener_top_level_frame_url,
                       const GURL& source_origin,
                       content::mojom::WindowContainerType container_type,
                       const GURL& target_url,
                       const content::Referrer& referrer,
                       const std::string& frame_name,
                       WindowOpenDisposition disposition,
                       const blink::mojom::WindowFeatures& features,
                       bool user_gesture,
                       bool opener_suppressed,
                       content::ResourceContext* context,
                       bool* no_javascript_access) override;
  void ResourceDispatcherHostCreated() override;
  void OverrideWebkitPrefs(content::RenderViewHost* render_view_host,
                           content::WebPreferences* prefs) override;
  content::DevToolsManagerDelegate* GetDevToolsManagerDelegate() override;
  void RegisterInProcessServices(StaticServiceMap* services) override;
  void DidCreatePpapiPlugin(content::BrowserPpapiHost* browser_host) override;
  gpu::GpuControlList::OsType GetOsTypeOverrideForGpuDataManager(
      std::string* os_version) override;

  std::string application_locale_;
  std::unique_ptr<BrowserPlatformIntegration> platform_integration_;

  std::unique_ptr<ResourceDispatcherHostDelegate>
      resource_dispatcher_host_delegate_;

  DISALLOW_COPY_AND_ASSIGN(ContentBrowserClient);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_CONTENT_BROWSER_CLIENT_H_
