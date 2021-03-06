// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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
#include "content/public/browser/certificate_request_result_type.h"
#include "content/public/browser/interstitial_page.h"
#include "content/public/browser/permission_type.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/service_info.h"
#include "content/public/common/web_preferences.h"
#include "ppapi/features/features.h"
#include "services/device/public/interfaces/constants.mojom.h"
#include "services/service_manager/public/cpp/interface_registry.h"
#include "ui/display/display.h"
#include "ui/native_theme/native_theme_switches.h"

#include "shared/browser/compositor/oxide_compositor_utils.h"
#include "shared/browser/device/oxide_device_service.h"
#include "shared/browser/media/oxide_media_capture_devices_dispatcher.h"
#include "shared/browser/notifications/oxide_platform_notification_service.h"
#include "shared/browser/ssl/oxide_certificate_error_dispatcher.h"
#include "shared/common/oxide_constants.h"
#include "shared/common/oxide_content_client.h"

#include "display_form_factor.h"
#include "in_process_renderer_observer.h"
#include "oxide_browser_context.h"
#include "oxide_browser_context_destroyer.h"
#include "oxide_browser_main_parts.h"
#include "oxide_browser_platform_integration.h"
#include "oxide_browser_process_main.h"
#include "oxide_devtools_manager_delegate.h"
#include "oxide_quota_permission_context.h"
#include "oxide_render_message_filter.h"
#include "oxide_resource_dispatcher_host_delegate.h"
#include "oxide_user_agent_settings.h"
#include "oxide_web_contents_view.h"
#include "screen.h"
#include "shell_mode.h"
#include "web_contents_client.h"
#include "web_contents_helper.h"
#include "web_preferences.h"

#if defined(ENABLE_HYBRIS)
#include "hybris_utils.h"
#endif

#if BUILDFLAG(ENABLE_PLUGINS)
#include "content/public/browser/browser_ppapi_host.h"
#include "ppapi/host/ppapi_host.h"

#include "pepper/oxide_pepper_host_factory_browser.h"
#endif

namespace oxide {

content::BrowserMainParts* ContentBrowserClient::CreateBrowserMainParts(
    const content::MainFunctionParams& parameters) {
  return new BrowserMainParts();
}

void ContentBrowserClient::RenderProcessWillLaunch(
    content::RenderProcessHost* host) {
  if (content::RenderProcessHost::run_renderer_in_process()) {
    host->AddObserver(new InProcessRendererObserver());
  }

  host->AddFilter(new RenderMessageFilter(host));
}

void ContentBrowserClient::SiteInstanceGotProcess(
    content::SiteInstance* site_instance) {
  BrowserContextDestroyer::RenderProcessHostAssignedToSiteInstance(
      site_instance->GetProcess());
}

void ContentBrowserClient::AppendExtraCommandLineSwitches(
    base::CommandLine* command_line,
    int child_process_id) {
  // This can be called on the UI or IO thread
  static const char* const kSwitchNames[] = {
    switches::kEnableMediaHubAudio,
    switches::kMediaHubFixedSessionDomains,
    switches::kSharedMemoryOverridePath
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

std::string
ContentBrowserClient::GetApplicationLocale() {
  return application_locale_;
}

std::string ContentBrowserClient::GetAcceptLangs(
    content::BrowserContext* browser_context) {
  return UserAgentSettings::Get(browser_context)->GetAcceptLangs();
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
    const base::Callback<void(content::CertificateRequestResultType)>& callback) {
  CertificateErrorDispatcher::AllowCertificateError(contents,
                                                    is_main_frame,
                                                    cert_error,
                                                    ssl_info,
                                                    request_url,
                                                    resource_type,
                                                    overridable,
                                                    strict_enforcement,
                                                    callback);
}

content::MediaObserver* ContentBrowserClient::GetMediaObserver() {
  return MediaCaptureDevicesDispatcher::GetInstance();
}

content::PlatformNotificationService*
ContentBrowserClient::GetPlatformNotificationService() {
  return PlatformNotificationService::GetInstance();
}

bool ContentBrowserClient::CanCreateWindow(
    int opener_render_process_id,
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
    bool* no_javascript_access) {
  *no_javascript_access = false;

  if (user_gesture) {
    return true;
  }

  return !BrowserContextIOData::FromResourceContext(
      context)->GetUserAgentSettings()->IsPopupBlockerEnabled();
}

void ContentBrowserClient::ResourceDispatcherHostCreated() {
  resource_dispatcher_host_delegate_.reset(new ResourceDispatcherHostDelegate());
  content::ResourceDispatcherHost::Get()->SetDelegate(
      resource_dispatcher_host_delegate_.get());
}

void ContentBrowserClient::OverrideWebkitPrefs(
    content::RenderViewHost* render_view_host,
    content::WebPreferences* prefs) {
  content::WebContents* contents =
      content::WebContents::FromRenderViewHost(render_view_host);
  if (!contents) {
    content::InterstitialPage* interstitial =
        content::InterstitialPage::FromRenderFrameHost(
            render_view_host->GetMainFrame());
    if (interstitial) {
      contents = interstitial->GetWebContents();
    }
  }

  DCHECK(contents);

  WebContentsHelper* contents_helper =
      WebContentsHelper::FromWebContents(contents);
  DCHECK(contents_helper) <<
      "WebContents must be created with WebContentsHelper::CreateWebContents";

  contents_helper->GetPreferences()->ApplyToWebkitPrefs(prefs);

  prefs->touch_event_feature_detection_enabled = true;
  prefs->device_supports_touch = platform_integration_->IsTouchSupported();

  prefs->javascript_can_open_windows_automatically =
      !UserAgentSettings::Get(
        render_view_host->GetProcess()->GetBrowserContext())
        ->IsPopupBlockerEnabled();

  prefs->allow_custom_scrollbar_in_main_frame = false;
  prefs->double_tap_to_zoom_enabled = true;
  prefs->viewport_meta_enabled = true;

  display::Display display;
  if (contents) {
    display = WebContentsView::FromWebContents(contents)->GetDisplay();
  } else {
    display = Screen::GetInstance()->GetPrimaryDisplay();
  }

  DisplayFormFactor form_factor =
      Screen::GetInstance()->GetDisplayFormFactor(display);
  switch (form_factor) {
    case DisplayFormFactor::Monitor:
      prefs->default_minimum_page_scale_factor = 1.0f;
      prefs->default_maximum_page_scale_factor = 4.f;
      prefs->viewport_style = content::ViewportStyle::DEFAULT;
      break;
    case DisplayFormFactor::Mobile:
      prefs->default_minimum_page_scale_factor = 0.25f;
      prefs->default_maximum_page_scale_factor = 5.f;
      prefs->viewport_style = content::ViewportStyle::MOBILE;
      break;
    case DisplayFormFactor::Television:
      prefs->default_minimum_page_scale_factor = 0.25f;
      prefs->default_maximum_page_scale_factor = 5.f;
      prefs->viewport_style = content::ViewportStyle::TELEVISION;
      break;
  };

  if (Screen::GetInstance()->GetShellMode() == ShellMode::NonWindowed) {
    prefs->shrinks_viewport_contents_to_fit = true;
    prefs->viewport_enabled = true;
    prefs->main_frame_resizes_are_orientation_changes = true;
  } else {
    prefs->shrinks_viewport_contents_to_fit = false;
    prefs->default_minimum_page_scale_factor = 1.0f;
    prefs->viewport_enabled = false;
    prefs->main_frame_resizes_are_orientation_changes = false;
  }

  prefs->supports_multiple_windows = false;
  if (contents_helper->client()) {
    prefs->supports_multiple_windows =
        contents_helper->client()->CanCreateWindows();
  }

  prefs->use_solid_color_scrollbars = ui::IsOverlayScrollbarEnabled();
}

content::DevToolsManagerDelegate*
ContentBrowserClient::GetDevToolsManagerDelegate() {
  return new DevToolsManagerDelegate();
}

void ContentBrowserClient::RegisterInProcessServices(
    StaticServiceMap* services) {
  content::ServiceInfo device_info;
  device_info.factory =
      base::Bind(
          &CreateDeviceService,
          content::BrowserThread::GetTaskRunnerForThread(
              content::BrowserThread::FILE),
          content::BrowserThread::GetTaskRunnerForThread(
              content::BrowserThread::IO));
  device_info.task_runner = base::ThreadTaskRunnerHandle::Get();
  services->insert(std::make_pair(device::mojom::kServiceName, device_info));
}

void ContentBrowserClient::DidCreatePpapiPlugin(content::BrowserPpapiHost* host) {
#if BUILDFLAG(ENABLE_PLUGINS)
  host->GetPpapiHost()->AddHostFactoryFilter(
      base::WrapUnique(new PepperHostFactoryBrowser(host)));
#endif
}

gpu::GpuControlList::OsType
ContentBrowserClient::GetOsTypeOverrideForGpuDataManager(
    std::string* os_version) {
#if defined(ENABLE_HYBRIS)
  if (!HybrisUtils::GetInstance()->IsUsingAndroidEGL()) {
    // Use the platform defaults in this case
    return gpu::GpuControlList::kOsAny;
  }

  *os_version = HybrisUtils::GetInstance()->GetDeviceProperties().os_version;
  return gpu::GpuControlList::kOsAndroid;
#else
  return gpu::GpuControlList::kOsAny;
#endif
}

ContentBrowserClient::ContentBrowserClient(
    const std::string& application_locale,
    BrowserPlatformIntegration* integration)
    : application_locale_(application_locale),
      platform_integration_(integration) {}

ContentBrowserClient::~ContentBrowserClient() {}

} // namespace oxide
