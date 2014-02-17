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

#include <vector>

#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "content/browser/loader/resource_dispatcher_host_impl.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/browser/render_process_host.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"
#include "ui/gfx/display.h"
#include "ui/gfx/ozone/surface_factory_ozone.h"
#include "ui/gfx/screen.h"
#include "ui/gfx/screen_type_delegate.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_share_group.h"
#include "webkit/common/webpreferences.h"

#include "shared/common/oxide_content_client.h"
#include "shared/common/oxide_messages.h"
#include "shared/gl/oxide_shared_gl_context.h"

#include "oxide_browser_context.h"
#include "oxide_browser_process_main.h"
#include "oxide_default_screen_info.h"
#include "oxide_message_dispatcher_browser.h"
#include "oxide_message_pump.h"
#include "oxide_web_contents_view.h"
#include "oxide_web_preferences.h"
#include "oxide_web_view.h"

namespace oxide {

namespace {

base::MessagePump* CreateMessagePumpForUI() {
  return ContentClient::instance()->browser()->
      CreateMessagePumpForUI();
}

class BrowserMainParts;
BrowserMainParts* g_main_parts;

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
  BrowserMainParts() {
    DCHECK(!g_main_parts);
    g_main_parts = this;
  }

  ~BrowserMainParts() {
    DCHECK_EQ(g_main_parts, this);
    g_main_parts = NULL;
  }

  void PreEarlyInitialization() FINAL {
    base::MessageLoop::InitMessagePumpForUIFactory(CreateMessagePumpForUI);
    main_message_loop_.reset(new base::MessageLoop(base::MessageLoop::TYPE_UI));
  
  }

  int PreCreateThreads() FINAL {
    // Make sure we initialize the display handle on the main thread
    gfx::SurfaceFactoryOzone::GetInstance()->GetNativeDisplay();

    gfx::Screen::SetScreenInstance(gfx::SCREEN_TYPE_NATIVE,
                                   &primary_screen_);

    BrowserProcessMain::instance()->CreateIOThreadDelegate();
    return 0;
  }

  bool MainMessageLoopRun(int* result_code) FINAL {
    MessageLoopForUI::current()->Start();
    return true;
  }

  void PostDestroyThreads() FINAL {
    gfx::Screen::SetScreenInstance(gfx::SCREEN_TYPE_NATIVE, NULL);
  }

 private:
  scoped_ptr<base::MessageLoop> main_message_loop_;
  Screen primary_screen_;
};

} // namespace

ContentBrowserClient::ContentBrowserClient() {}

ContentBrowserClient::~ContentBrowserClient() {}

content::BrowserMainParts* ContentBrowserClient::CreateBrowserMainParts(
    const content::MainFunctionParams& parameters) {
  return new BrowserMainParts();
}

content::WebContentsViewPort*
ContentBrowserClient::OverrideCreateWebContentsView(
    content::WebContents* web_contents,
    content::RenderViewHostDelegateView** render_view_host_delegate_view) {
  WebContentsView* view = new WebContentsView(web_contents);
  *render_view_host_delegate_view = view;

  return view;
}

void ContentBrowserClient::RenderProcessWillLaunch(
    content::RenderProcessHost* host) {
  host->Send(new OxideMsg_SetIsIncognitoProcess(
      host->GetBrowserContext()->IsOffTheRecord()));
  host->AddFilter(new MessageDispatcherBrowser(host->GetID()));
}

net::URLRequestContextGetter* ContentBrowserClient::CreateRequestContext(
    content::BrowserContext* browser_context,
    content::ProtocolHandlerMap* protocol_handlers) {
  return BrowserContext::FromContent(
      browser_context)->CreateRequestContext(protocol_handlers);
}

net::URLRequestContextGetter*
ContentBrowserClient::CreateRequestContextForStoragePartition(
    content::BrowserContext* browser_context,
    const base::FilePath& partition_path,
    bool in_memory,
    content::ProtocolHandlerMap* protocol_handlers) {
  // We don't return any storage partition names from
  // GetStoragePartitionConfigForSite(), so it's a bug to hit this
  NOTREACHED() << "Invalid request for request context for storage partition";
  return NULL;
}

std::string ContentBrowserClient::GetAcceptLangs(
    content::BrowserContext* browser_context) {
  return BrowserContext::FromContent(browser_context)->GetAcceptLangs();
}

void ContentBrowserClient::ResourceDispatcherHostCreated() {
  std::vector<BrowserContext *>& contexts = BrowserContext::GetAllContexts();

  content::ResourceDispatcherHostImpl* rdhi =
      content::ResourceDispatcherHostImpl::Get();

  for (std::vector<BrowserContext *>::iterator it = contexts.begin();
       it != contexts.end(); ++it) {
    BrowserContext* c = *it;

    rdhi->AddResourceContext(c->GetResourceContext());
  }
}

void ContentBrowserClient::OverrideWebkitPrefs(
    content::RenderViewHost* render_view_host,
    const GURL& url,
    ::WebPreferences* prefs) {

  WebView* view = WebView::FromRenderViewHost(render_view_host);
  WebPreferences* web_prefs = view->GetWebPreferences();
  if (web_prefs) {
    web_prefs->ApplyToWebkitPrefs(prefs);
  }

  prefs->device_supports_mouse = true; // XXX: Can we detect this?
  prefs->device_supports_touch = prefs->touch_enabled && IsTouchSupported();
}

gfx::GLShareGroup* ContentBrowserClient::GetGLShareGroup() {
  SharedGLContext* context =
      BrowserProcessMain::instance()->shared_gl_context();
  if (!context) {
    return NULL;
  }

  return context->share_group();
}

WebPreferences* ContentBrowserClient::GetDefaultWebPreferences() {
  return NULL;
}

bool ContentBrowserClient::IsTouchSupported() {
  return false;
}

} // namespace oxide
