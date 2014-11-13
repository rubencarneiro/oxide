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

#include "oxide_content_browser_client.h"

#include <string>
#include <vector>

#include "base/command_line.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/browser/gpu/compositor_util.h"
#include "content/browser/gpu/gpu_data_manager_impl.h"
#include "content/browser/gpu/gpu_process_host.h"
#include "content/public/browser/certificate_request_result_type.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/web_preferences.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_share_group.h"

#include "shared/common/oxide_constants.h"
#include "shared/common/oxide_content_client.h"
#include "shared/common/oxide_messages.h"

#include "oxide_access_token_store.h"
#include "oxide_browser_context.h"
#include "oxide_browser_process_main.h"
#include "oxide_devtools_manager_delegate.h"
#include "oxide_form_factor.h"
#include "oxide_resource_dispatcher_host_delegate.h"
#include "oxide_script_message_dispatcher_browser.h"
#include "oxide_shared_gl_context.h"
#include "oxide_user_agent_override_provider.h"
#include "oxide_web_preferences.h"
#include "oxide_web_view.h"
#include "oxide_web_view_contents_helper.h"

#if defined(ENABLE_PLUGINS)
#include "content/public/browser/browser_ppapi_host.h"
#include "ppapi/host/ppapi_host.h"

#include "oxide_pepper_host_factory_browser.h"
#endif

namespace oxide {

content::BrowserMainParts* ContentBrowserClient::CreateBrowserMainParts(
    const content::MainFunctionParams& parameters) {
  return new BrowserMainParts(CreateBrowserMainPartsDelegate());
}

void ContentBrowserClient::RenderProcessWillLaunch(
    content::RenderProcessHost* host) {
  host->Send(new OxideMsg_SetUserAgent(
      BrowserContext::FromContent(host->GetBrowserContext())->GetUserAgent()));
  host->AddFilter(new ScriptMessageDispatcherBrowser(host));
  host->AddFilter(new UserAgentOverrideProvider(host));
}

net::URLRequestContextGetter* ContentBrowserClient::CreateRequestContext(
    content::BrowserContext* browser_context,
    content::ProtocolHandlerMap* protocol_handlers,
    content::URLRequestInterceptorScopedVector request_interceptors) {
  return BrowserContext::FromContent(
      browser_context)->CreateRequestContext(protocol_handlers,
                                             request_interceptors.Pass());
}

net::URLRequestContextGetter*
ContentBrowserClient::CreateRequestContextForStoragePartition(
    content::BrowserContext* browser_context,
    const base::FilePath& partition_path,
    bool in_memory,
    content::ProtocolHandlerMap* protocol_handlers,
    content::URLRequestInterceptorScopedVector request_interceptors) {
  // We don't return any storage partition names from
  // GetStoragePartitionConfigForSite(), so it's a bug to hit this
  NOTREACHED() << "Invalid request for request context for storage partition";
  return NULL;
}

std::string ContentBrowserClient::GetAcceptLangs(
    content::BrowserContext* browser_context) {
  return BrowserContext::FromContent(browser_context)->GetAcceptLangs();
}

void ContentBrowserClient::AppendExtraCommandLineSwitches(
    base::CommandLine* command_line,
    int child_process_id) {
  std::string process_type =
      command_line->GetSwitchValueASCII(switches::kProcessType);
  if (process_type == switches::kRendererProcess) {
    static const char* const kSwitchNames[] = {
      switches::kEnableGoogleTalkPlugin,
      switches::kFormFactor,
      switches::kEnableMediaHubAudio,
      switches::kMediaHubFixedSessionDomains
    };
    command_line->CopySwitchesFrom(*base::CommandLine::ForCurrentProcess(),
                                   kSwitchNames, arraysize(kSwitchNames));

    content::RenderProcessHost* host =
        content::RenderProcessHost::FromID(child_process_id);
    if (host->GetBrowserContext()->IsOffTheRecord()) {
      command_line->AppendSwitch(switches::kIncognito);
    }

    if (content::GpuDataManagerImpl::GetInstance()->CanUseGpuBrowserCompositor() &&
        (!BrowserProcessMain::GetInstance()->GetSharedGLContext() ||
         BrowserProcessMain::GetInstance()->GetSharedGLContext()->GetImplementation() !=
             gfx::GetGLImplementation())) {
      command_line->AppendSwitch(switches::kDisableGpuCompositing);
    }
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
                                          net::CookieOptions* options) {
  return BrowserContextIOData::FromResourceContext(
      context)->CanAccessCookies(url, first_party, true);
}

void ContentBrowserClient::AllowCertificateError(
    int render_process_id,
    int render_frame_id,
    int cert_error,
    const net::SSLInfo& ssl_info,
    const GURL& request_url,
    content::ResourceType resource_type,
    bool overridable,
    bool strict_enforcement,
    bool expired_previous_decision,
    const base::Callback<void(bool)>& callback,
    content::CertificateRequestResultType* result) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  content::RenderFrameHost* rfh = content::RenderFrameHost::FromID(
      render_process_id, render_frame_id);
  if (!rfh) {
    *result = content::CERTIFICATE_REQUEST_RESULT_TYPE_CANCEL;
    return;
  }

  WebView* webview = WebView::FromRenderFrameHost(rfh);
  if (!webview) {
    *result = content::CERTIFICATE_REQUEST_RESULT_TYPE_CANCEL;
    return;
  }

  webview->AllowCertificateError(rfh, cert_error, ssl_info, request_url,
                                 resource_type, overridable,
                                 strict_enforcement, callback,
                                 result);
}

void ContentBrowserClient::RequestGeolocationPermission(
    content::WebContents* web_contents,
    int bridge_id,
    const GURL& requesting_frame,
    bool user_gesture,
    const base::Callback<void(bool)>& result_callback) {
  WebView* webview = WebView::FromWebContents(web_contents);
  if (!webview) {
    result_callback.Run(false);
    return;
  }

  webview->RequestGeolocationPermission(requesting_frame.GetOrigin(),
                                        result_callback);
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
    int opener_id,
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
    const GURL& url,
    content::WebPreferences* prefs) {
  WebViewContentsHelper* contents_helper =
      WebViewContentsHelper::FromRenderViewHost(render_view_host);

  WebPreferences* web_prefs = contents_helper->GetWebPreferences();
  web_prefs->ApplyToWebkitPrefs(prefs);

  prefs->touch_enabled = true;
  prefs->device_supports_mouse = true; // XXX: Can we detect this?
  prefs->device_supports_touch = IsTouchSupported();

  prefs->javascript_can_open_windows_automatically =
      !contents_helper->GetBrowserContext()->IsPopupBlockerEnabled();

  FormFactor form_factor = GetFormFactorHint();
  if (form_factor == FORM_FACTOR_TABLET || form_factor == FORM_FACTOR_PHONE) {
    prefs->shrinks_standalone_images_to_fit = false;
  }

  prefs->supports_multiple_windows = false;
  WebView* view = WebView::FromRenderViewHost(render_view_host);
  if (view) {
    prefs->supports_multiple_windows = view->CanCreateWindows();
  }
}

content::DevToolsManagerDelegate*
ContentBrowserClient::GetDevToolsManagerDelegate() {
    return new DevToolsManagerDelegate();
}

gfx::GLShareGroup* ContentBrowserClient::GetGLShareGroup() {
  SharedGLContext* context =
      BrowserProcessMain::GetInstance()->GetSharedGLContext();
  if (!context) {
    return NULL;
  }

  return context->share_group();
}

void ContentBrowserClient::DidCreatePpapiPlugin(content::BrowserPpapiHost* host) {
#if defined(ENABLE_PLUGINS)
  host->GetPpapiHost()->AddHostFactoryFilter(
      scoped_ptr<ppapi::host::HostFactory>(new PepperHostFactoryBrowser(host)));
#endif
}

bool ContentBrowserClient::IsTouchSupported() {
  return false;
}

ContentBrowserClient::ContentBrowserClient() {}

ContentBrowserClient::~ContentBrowserClient() {}

} // namespace oxide
