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

#include "oxide_content_browser_client.h"

#include <string>
#include <utility>
#include <vector>

#include "base/command_line.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted.h"
#include "content/public/common/service_registry.h"
#include "content/public/browser/certificate_request_result_type.h"
#include "content/public/browser/geolocation_provider.h"
#include "content/public/browser/location_provider.h"
#include "content/public/browser/permission_type.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/web_preferences.h"
#include "device/vibration/vibration_manager_impl.h"

#include "shared/browser/compositor/oxide_compositor_utils.h"
#include "shared/browser/media/oxide_media_capture_devices_dispatcher.h"
#include "shared/browser/notifications/oxide_platform_notification_service.h"
#include "shared/browser/ssl/oxide_certificate_error_dispatcher.h"
#include "shared/common/oxide_constants.h"
#include "shared/common/oxide_content_client.h"
#include "shared/common/oxide_form_factor.h"

#include "oxide_access_token_store.h"
#include "oxide_browser_context.h"
#include "oxide_browser_main_parts.h"
#include "oxide_browser_platform_integration.h"
#include "oxide_browser_process_main.h"
#include "oxide_quota_permission_context.h"
#include "oxide_render_message_filter.h"
#include "oxide_resource_dispatcher_host_delegate.h"
#include "oxide_user_agent_settings.h"
#include "oxide_web_preferences.h"
#include "oxide_web_view.h"
#include "oxide_web_view_contents_helper.h"

#if defined(ENABLE_HYBRIS)
#include "oxide_hybris_utils.h"
#endif

#if defined(ENABLE_PLUGINS)
#include "content/public/browser/browser_ppapi_host.h"
#include "ppapi/host/ppapi_host.h"

#include "pepper/oxide_pepper_host_factory_browser.h"
#endif

namespace oxide {

namespace {

void CreateVibrationManager(
    mojo::InterfaceRequest<device::VibrationManager> request) {
  BrowserPlatformIntegration::GetInstance()
      ->CreateVibrationManager(std::move(request));
}

}

content::BrowserMainParts* ContentBrowserClient::CreateBrowserMainParts(
    const content::MainFunctionParams& parameters) {
  return new BrowserMainParts();
}

void ContentBrowserClient::RenderProcessWillLaunch(
    content::RenderProcessHost* host) {
  host->AddFilter(new RenderMessageFilter(host));
}

std::string ContentBrowserClient::GetAcceptLangs(
    content::BrowserContext* browser_context) {
  return UserAgentSettings::Get(browser_context)->GetAcceptLangs();
}

void ContentBrowserClient::AppendExtraCommandLineSwitches(
    base::CommandLine* command_line,
    int child_process_id) {
  // This can be called on the UI or IO thread
  static const char* const kSwitchNames[] = {
    switches::kEnableMediaHubAudio,
    switches::kFormFactor,
    switches::kMediaHubFixedSessionDomains
  };
  command_line->CopySwitchesFrom(*base::CommandLine::ForCurrentProcess(),
                                 kSwitchNames, arraysize(kSwitchNames));

  std::string process_type =
      command_line->GetSwitchValueASCII(switches::kProcessType);
  if (process_type == switches::kRendererProcess) {
    // For renderer processes, we should always be on the UI thread
    DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

    content::RenderProcessHost* host =
        content::RenderProcessHost::FromID(child_process_id);
    if (host->GetBrowserContext()->IsOffTheRecord()) {
      command_line->AppendSwitch(switches::kIncognito);
    }
  }

  if (!CompositorUtils::GetInstance()->CanUseGpuCompositing()) {
    command_line->AppendSwitch(switches::kDisableGpuCompositing);
  }
}

bool ContentBrowserClient::AllowGetCookie(const GURL& url,
                                          const GURL& first_party,
                                          const net::CookieList& cookie_list,
                                          content::ResourceContext* context,
                                          int render_process_id,
                                          int render_frame_id) {
  return BrowserContextIOData::FromResourceContext(
      context)->CanAccessCookies(url, first_party, false);
}

bool ContentBrowserClient::AllowSetCookie(const GURL& url,
                                          const GURL& first_party,
                                          const std::string& cookie_line,
                                          content::ResourceContext* context,
                                          int render_process_id,
                                          int render_frame_id,
                                          const net::CookieOptions& options) {
  return BrowserContextIOData::FromResourceContext(
      context)->CanAccessCookies(url, first_party, true);
}

content::QuotaPermissionContext* ContentBrowserClient::CreateQuotaPermissionContext() {
  return new QuotaPermissionContext();
}

void ContentBrowserClient::AllowCertificateError(
    content::WebContents* contents,
    bool is_main_frame,
    int cert_error,
    const net::SSLInfo& ssl_info,
    const GURL& request_url,
    content::ResourceType resource_type,
    bool overridable,
    bool strict_enforcement,
    bool expired_previous_decision,
    const base::Callback<void(bool)>& callback,
    content::CertificateRequestResultType* result) {
  CertificateErrorDispatcher::AllowCertificateError(contents,
                                                    is_main_frame,
                                                    cert_error,
                                                    ssl_info,
                                                    request_url,
                                                    resource_type,
                                                    overridable,
                                                    strict_enforcement,
                                                    callback,
                                                    result);
}

content::MediaObserver* ContentBrowserClient::GetMediaObserver() {
  return MediaCaptureDevicesDispatcher::GetInstance();
}

content::PlatformNotificationService*
ContentBrowserClient::GetPlatformNotificationService() {
  return PlatformNotificationService::GetInstance();
}

bool ContentBrowserClient::CanCreateWindow(
    const GURL& opener_url,
    const GURL& opener_top_level_frame_url,
    const GURL& source_origin,
    WindowContainerType container_type,
    const GURL& target_url,
    const content::Referrer& referrer,
    WindowOpenDisposition disposition,
    const blink::WebWindowFeatures& features,
    bool user_gesture,
    bool opener_suppressed,
    content::ResourceContext* context,
    int render_process_id,
    int opener_render_view_id,
    int opener_render_frame_id,
    bool* no_javascript_access) {
  *no_javascript_access = false;

  if (user_gesture) {
    return true;
  }

  return !BrowserContextIOData::FromResourceContext(
      context)->IsPopupBlockerEnabled();
}

void ContentBrowserClient::ResourceDispatcherHostCreated() {
  resource_dispatcher_host_delegate_.reset(new ResourceDispatcherHostDelegate());
  content::ResourceDispatcherHost::Get()->SetDelegate(
      resource_dispatcher_host_delegate_.get());
}

content::AccessTokenStore* ContentBrowserClient::CreateAccessTokenStore() {
  return new AccessTokenStore();
}

void ContentBrowserClient::OverrideWebkitPrefs(
    content::RenderViewHost* render_view_host,
    content::WebPreferences* prefs) {
  WebViewContentsHelper* contents_helper =
      WebViewContentsHelper::FromRenderViewHost(render_view_host);

  WebPreferences* web_prefs = nullptr;
  if (contents_helper) {
    // If RVH is for an InterstitialPage, we can't map to WebContents
    // XXX: If we ever expose transient pages in the public API, we should find
    //  a way around this, so that transient pages get the same preferences
    web_prefs = contents_helper->GetWebPreferences();
  }
  if (!web_prefs) {
    web_prefs = WebPreferences::GetFallback();
  }

  web_prefs->ApplyToWebkitPrefs(prefs);

  prefs->touch_enabled = true;
  prefs->device_supports_mouse = true; // XXX: Can we detect this?
  prefs->device_supports_touch = platform_integration_->IsTouchSupported();

  prefs->javascript_can_open_windows_automatically =
      !BrowserContext::FromContent(
        render_view_host->GetProcess()->GetBrowserContext())
        ->IsPopupBlockerEnabled();

  FormFactor form_factor = GetFormFactorHint();
  if (form_factor == FORM_FACTOR_TABLET || form_factor == FORM_FACTOR_PHONE) {
    prefs->shrinks_standalone_images_to_fit = false;
    prefs->default_minimum_page_scale_factor = 0.25f;
    prefs->default_maximum_page_scale_factor = 5.f;
    prefs->viewport_meta_enabled = true;
    prefs->viewport_style = content::ViewportStyle::MOBILE;
  }

  prefs->supports_multiple_windows = false;
  WebView* view = WebView::FromRenderViewHost(render_view_host);
  if (view) {
    prefs->supports_multiple_windows = view->CanCreateWindows();
  }
}

content::LocationProvider*
ContentBrowserClient::OverrideSystemLocationProvider() {
  return platform_integration_->CreateLocationProvider().release();
}

void ContentBrowserClient::RegisterRenderFrameMojoServices(
    content::ServiceRegistry* registry,
    content::RenderFrameHost* render_frame_host) {
  DCHECK(registry);
  registry->AddService(base::Bind(&CreateVibrationManager));
}

void ContentBrowserClient::DidCreatePpapiPlugin(content::BrowserPpapiHost* host) {
#if defined(ENABLE_PLUGINS)
  host->GetPpapiHost()->AddHostFactoryFilter(
      base::WrapUnique(new PepperHostFactoryBrowser(host)));
#endif
}

gpu::GpuControlList::OsType
ContentBrowserClient::GetOsTypeOverrideForGpuDataManager(
    std::string* os_version) {
#if defined(ENABLE_HYBRIS)
  if (!HybrisUtils::IsUsingAndroidEGL()) {
    // Use the platform defaults in this case
    return gpu::GpuControlList::kOsAny;
  }

  *os_version = HybrisUtils::GetDeviceProperties().os_version;
  return gpu::GpuControlList::kOsAndroid;
#else
  return gpu::GpuControlList::kOsAny;
#endif
}

std::string
ContentBrowserClient::GetApplicationLocale() {
  return application_locale_;
}

ContentBrowserClient::ContentBrowserClient(
    const std::string& application_locale,
    BrowserPlatformIntegration* integration)
    : application_locale_(application_locale),
      platform_integration_(integration) {}

ContentBrowserClient::~ContentBrowserClient() {}

} // namespace oxide
