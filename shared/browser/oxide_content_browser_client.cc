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
#include "base/message_loop/message_loop.h"
#include "content/browser/gpu/compositor_util.h"
#include "content/browser/gpu/gpu_process_host.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/common/content_switches.h"
#include "net/base/net_module.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"
#include "ui/gfx/display.h"
#include "ui/gfx/ozone/surface_factory_ozone.h"
#include "ui/gfx/screen.h"
#include "ui/gfx/screen_type_delegate.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_share_group.h"
#include "webkit/common/webpreferences.h"

#include "shared/common/oxide_constants.h"
#include "shared/common/oxide_content_client.h"
#include "shared/common/oxide_messages.h"
#include "shared/common/oxide_net_resource_provider.h"
#include "shared/gl/oxide_shared_gl_context.h"

#include "oxide_access_token_store.h"
#include "oxide_browser_context.h"
#include "oxide_browser_process_main.h"
#include "oxide_default_screen_info.h"
#include "oxide_form_factor.h"
#include "oxide_gpu_utils.h"
#include "oxide_io_thread.h"
#include "oxide_message_pump.h"
#include "oxide_script_message_dispatcher_browser.h"
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

namespace {

scoped_ptr<base::MessagePump> CreateMessagePumpForUI() {
  return make_scoped_ptr(
      ContentClient::instance()->browser()->CreateMessagePumpForUI());
}

class Screen : public gfx::Screen {
 public:
  Screen() {}

  bool IsDIPEnabled() FINAL {
    NOTIMPLEMENTED();
    return true;
  }

  gfx::Point GetCursorScreenPoint() FINAL {
    NOTIMPLEMENTED();
    return gfx::Point();
  }

  gfx::NativeWindow GetWindowUnderCursor() FINAL {
    NOTIMPLEMENTED();
    return NULL;
  }

  gfx::NativeWindow GetWindowAtScreenPoint(const gfx::Point& point) FINAL {
    NOTIMPLEMENTED();
    return NULL;
  }

  int GetNumDisplays() const FINAL {
    NOTIMPLEMENTED();
    return 1;
  }

  std::vector<gfx::Display> GetAllDisplays() const FINAL {
    NOTIMPLEMENTED();
    return std::vector<gfx::Display>();
  }

  gfx::Display GetDisplayNearestWindow(gfx::NativeView view) const FINAL {
    NOTIMPLEMENTED();
    return gfx::Display();
  }

  gfx::Display GetDisplayNearestPoint(const gfx::Point& point) const FINAL {
    NOTIMPLEMENTED();
    return gfx::Display();
  }

  gfx::Display GetDisplayMatching(const gfx::Rect& match_rect) const FINAL {
    NOTIMPLEMENTED();
    return gfx::Display();
  }

  gfx::Display GetPrimaryDisplay() const FINAL {
    blink::WebScreenInfo info(GetDefaultWebScreenInfo());

    gfx::Display display;
    display.set_bounds(info.rect);
    display.set_work_area(info.availableRect);
    display.set_device_scale_factor(info.deviceScaleFactor);

    return display;
  }

  void AddObserver(gfx::DisplayObserver* observer) FINAL {
    NOTIMPLEMENTED();
  }
  void RemoveObserver(gfx::DisplayObserver* observer) FINAL {
    NOTIMPLEMENTED();
  }
};

class BrowserMainParts : public content::BrowserMainParts {
 public:
  BrowserMainParts() {}
  ~BrowserMainParts() {}

  void PreEarlyInitialization() FINAL {
    base::MessageLoop::InitMessagePumpForUIFactory(CreateMessagePumpForUI);
    main_message_loop_.reset(new base::MessageLoop(base::MessageLoop::TYPE_UI));
  }

  int PreCreateThreads() FINAL {
    gfx::Screen::SetScreenInstance(gfx::SCREEN_TYPE_NATIVE,
                                   &primary_screen_);

    io_thread_.reset(new IOThread());

    return 0;
  }

  void PreMainMessageLoopRun() FINAL {
    GpuUtils::Initialize();
    net::NetModule::SetResourceProvider(NetResourceProvider);
  }

  bool MainMessageLoopRun(int* result_code) FINAL {
    MessageLoopForUI::current()->Start();
    return true;
  }

  void PostMainMessageLoopRun() FINAL {
    GpuUtils::instance()->Destroy();
  }

  void PostDestroyThreads() FINAL {
    gfx::Screen::SetScreenInstance(gfx::SCREEN_TYPE_NATIVE, NULL);
    io_thread_.reset();
  }

 private:
  scoped_ptr<base::MessageLoop> main_message_loop_;
  scoped_ptr<IOThread> io_thread_;
  Screen primary_screen_;
};

} // namespace

content::BrowserMainParts* ContentBrowserClient::CreateBrowserMainParts(
    const content::MainFunctionParams& parameters) {
  return new BrowserMainParts();
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
    content::ProtocolHandlerScopedVector protocol_interceptors) {
  return BrowserContext::FromContent(
      browser_context)->CreateRequestContext(protocol_handlers,
                                             protocol_interceptors.Pass());
}

net::URLRequestContextGetter*
ContentBrowserClient::CreateRequestContextForStoragePartition(
    content::BrowserContext* browser_context,
    const base::FilePath& partition_path,
    bool in_memory,
    content::ProtocolHandlerMap* protocol_handlers,
    content::ProtocolHandlerScopedVector protocol_interceptors) {
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
      switches::kFormFactor
    };
    command_line->CopySwitchesFrom(*base::CommandLine::ForCurrentProcess(),
                                   kSwitchNames, arraysize(kSwitchNames));

    content::RenderProcessHost* host =
        content::RenderProcessHost::FromID(child_process_id);
    if (host->GetBrowserContext()->IsOffTheRecord()) {
      command_line->AppendSwitch(switches::kIncognito);
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
    bool is_guest,
    int opener_id,
    bool* no_javascript_access) {
  *no_javascript_access = false;

  if (user_gesture) {
    return true;
  }

  return !BrowserContextIOData::FromResourceContext(
      context)->IsPopupBlockerEnabled();
}

content::AccessTokenStore* ContentBrowserClient::CreateAccessTokenStore() {
  return new AccessTokenStore();
}

void ContentBrowserClient::OverrideWebkitPrefs(
    content::RenderViewHost* render_view_host,
    const GURL& url,
    ::WebPreferences* prefs) {
  WebViewContentsHelper* contents_helper =
      WebViewContentsHelper::FromRenderViewHost(render_view_host);

  WebPreferences* web_prefs = contents_helper->GetWebPreferences();
  web_prefs->ApplyToWebkitPrefs(prefs);

  prefs->touch_enabled = true;
  prefs->device_supports_mouse = true; // XXX: Can we detect this?
  prefs->device_supports_touch = IsTouchSupported();

  prefs->javascript_can_open_windows_automatically =
      !contents_helper->GetBrowserContext()->IsPopupBlockerEnabled();

  prefs->enable_scroll_animator = true;

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

content::LocationProvider*
ContentBrowserClient::OverrideSystemLocationProvider() {
  return NULL;
}

gfx::GLShareGroup* ContentBrowserClient::GetGLShareGroup() {
  SharedGLContext* context =
      BrowserProcessMain::instance()->GetSharedGLContext();
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
