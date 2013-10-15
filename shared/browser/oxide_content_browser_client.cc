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
#include "content/browser/loader/resource_dispatcher_host_impl.h"
#include "content/public/browser/render_process_host.h"

#include "shared/common/oxide_messages.h"

#include "oxide_browser_context.h"
#include "oxide_browser_main_parts.h"
#include "oxide_web_contents_view.h"

namespace oxide {

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

bool ContentBrowserClient::GetDefaultScreenInfo(
    WebKit::WebScreenInfo* result) {
  GetDefaultScreenInfoImpl(result);
  return true;
}

WebFrameTree* ContentBrowserClient::CreateWebFrameTree(
    content::RenderViewHost* rvh) {
  return NULL;
}

} // namespace oxide
