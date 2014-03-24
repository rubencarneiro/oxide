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

#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/content_browser_client.h"

namespace base {
class MessagePump;
}

namespace content {
class RenderViewHost;
}

namespace gfx {
class GLContext;
}

namespace oxide {

class GLShareGroup;
class SharedGLContext;
class WebFrameTree;

class ContentBrowserClient : public content::ContentBrowserClient {
 public:
  virtual ~ContentBrowserClient();

  virtual base::MessagePump* CreateMessagePumpForUI() = 0;

 protected:
  // Limit default constructor access to derived classes
  ContentBrowserClient();

 private:
  content::BrowserMainParts* CreateBrowserMainParts(
      const content::MainFunctionParams& parameters) FINAL;

  content::WebContentsViewPort* OverrideCreateWebContentsView(
      content::WebContents* web_contents,
      content::RenderViewHostDelegateView** render_view_host_delegate_view)
      FINAL;

  void RenderProcessWillLaunch(content::RenderProcessHost* host) FINAL;

  net::URLRequestContextGetter* CreateRequestContext(
      content::BrowserContext* browser_context,
      content::ProtocolHandlerMap* protocol_handlers,
      content::ProtocolHandlerScopedVector protocol_interceptors) FINAL;

  net::URLRequestContextGetter*
      CreateRequestContextForStoragePartition(
        content::BrowserContext* browser_context,
        const base::FilePath& partition_path,
        bool in_memory,
        content::ProtocolHandlerMap* protocol_handlers,
        content::ProtocolHandlerScopedVector protocol_interceptors) FINAL;

  std::string GetAcceptLangs(
      content::BrowserContext* browser_context) FINAL;

  bool AllowGetCookie(const GURL& url,
                      const GURL& first_party,
                      const net::CookieList& cookie_list,
                      content::ResourceContext* context,
                      int render_process_id,
                      int render_frame_id) FINAL;

  bool AllowSetCookie(const GURL& url,
                      const GURL& first_party,
                      const std::string& cookie_line,
                      content::ResourceContext* context,
                      int render_process_id,
                      int render_frame_id,
                      net::CookieOptions* options) FINAL;

  void ResourceDispatcherHostCreated() FINAL;

  content::AccessTokenStore* CreateAccessTokenStore() FINAL;

  void OverrideWebkitPrefs(content::RenderViewHost* render_view_host,
                           const GURL& url,
                           ::WebPreferences* prefs) FINAL;

  gfx::GLShareGroup* GetGLShareGroup() FINAL;

  virtual bool IsTouchSupported();

  DISALLOW_COPY_AND_ASSIGN(ContentBrowserClient);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_CONTENT_BROWSER_CLIENT_H_
