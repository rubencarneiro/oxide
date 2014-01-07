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
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_share_group.h"
#include "webkit/common/webpreferences.h"

#include "shared/common/oxide_content_client.h"
#include "shared/common/oxide_messages.h"
#include "shared/gl/oxide_shared_gl_context.h"

#include "oxide_browser_context.h"
#include "oxide_browser_process_main.h"
#include "oxide_message_pump.h"
#include "oxide_web_contents_view.h"

namespace oxide {

namespace {

base::MessagePump* CreateMessagePumpForUI() {
  return ContentClient::instance()->browser()->
      CreateMessagePumpForUI();
}

class BrowserMainParts;
BrowserMainParts* g_main_parts;

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
  
    scoped_refptr<oxide::GLShareGroup> share_group = new oxide::GLShareGroup();
    shared_gl_context_ =
        ContentClient::instance()->browser()->CreateSharedGLContext(
          share_group);
    if (shared_gl_context_) {
      DCHECK_EQ(shared_gl_context_->share_group(), share_group);
    }
  }

  int PreCreateThreads() FINAL {
    BrowserProcessMain::CreateIOThreadDelegate();
    return 0;
  }

  bool MainMessageLoopRun(int* result_code) FINAL {
    MessageLoopForUI::current()->Start();
    return true;
  }

  gfx::GLContext* shared_gl_context() const {
    return shared_gl_context_;
  }

 private:
  scoped_ptr<base::MessageLoop> main_message_loop_;
  scoped_refptr<gfx::GLContext> shared_gl_context_;
};

} // namespace

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

void ContentBrowserClient::RenderProcessHostCreated(
    content::RenderProcessHost* host) {
  host->Send(new OxideMsg_SetIsIncognitoProcess(
      host->GetBrowserContext()->IsOffTheRecord()));
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
    WebPreferences* prefs) {
  // XXX: This is temporary until we expose a WebPreferences API
  if (getenv("OXIDE_ENABLE_COMPOSITING") &&
      g_main_parts->shared_gl_context()) {
    prefs->force_compositing_mode = true;
    prefs->accelerated_compositing_enabled = true;
  } else {
    prefs->force_compositing_mode = false;
    prefs->accelerated_compositing_enabled = false;
  }
}

bool ContentBrowserClient::GetDefaultScreenInfo(
    blink::WebScreenInfo* result) {
  GetDefaultScreenInfoImpl(result);
  return true;
}

gfx::GLShareGroup* ContentBrowserClient::GetGLShareGroup() {
  if (!g_main_parts->shared_gl_context()) {
    DLOG(INFO) << "No shared GL context has been created. "
               << "Compositing will not work";
    return NULL;
  }

  return g_main_parts->shared_gl_context()->share_group();
}

scoped_refptr<gfx::GLContext> ContentBrowserClient::CreateSharedGLContext(
    oxide::GLShareGroup* share_group) {
  return scoped_refptr<gfx::GLContext>(NULL);
}

} // namespace oxide
