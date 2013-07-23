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

#include "oxide_browser_context.h"

#include "base/files/file_path.h"
#include "base/logging.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"

#include "public/browser/oxide_global_settings.h"

#include "oxide_browser_context_impl.h"
#include "oxide_browser_process_main.h"
#include "oxide_io_thread_delegate.h"
#include "oxide_off_the_record_browser_context_impl.h"
#include "oxide_url_request_context.h"

namespace {
oxide::BrowserContext* g_browser_context;
}

namespace oxide {

class ResourceContext FINAL : public content::ResourceContext {
 public:
  ResourceContext() :
      request_context_getter_(NULL) {}

  net::HostResolver* GetHostResolver() FINAL {
    return BrowserProcessMain::io_thread_delegate()->host_resolver();
  }

  net::URLRequestContext* GetRequestContext() FINAL {
    CHECK(request_context_getter_);
    return request_context_getter_->GetURLRequestContext();
  }

 private:
  friend class BrowserContext;

  URLRequestContextGetter* request_context_getter_;

  DISALLOW_COPY_AND_ASSIGN(ResourceContext);
};

BrowserContext::BrowserContext() :
    resource_context_(new ResourceContext()) {}

BrowserContext::~BrowserContext() {}

net::URLRequestContextGetter* BrowserContext::CreateRequestContext(
    content::ProtocolHandlerMap* protocol_handlers) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  DCHECK(!main_request_context_getter_);

  main_request_context_getter_ =
      URLRequestContextGetter::Create(
        protocol_handlers,
        GetPath(),
        base::FilePath(GlobalSettings::GetCachePath()),
        IsOffTheRecord());
  resource_context_->request_context_getter_ = main_request_context_getter_;

  return main_request_context_getter_;
}

net::URLRequestContextGetter* BrowserContext::GetRequestContext() {
  return GetStoragePartition(this, NULL)->GetURLRequestContext();
}

net::URLRequestContextGetter*
BrowserContext::GetRequestContextForRenderProcess(int renderer_child_id) {
  content::RenderProcessHost* host =
      content::RenderProcessHost::FromID(renderer_child_id);

  return host->GetStoragePartition()->GetURLRequestContext();
}

net::URLRequestContextGetter* BrowserContext::GetMediaRequestContext() {
  return GetRequestContext();
}

net::URLRequestContextGetter*
BrowserContext::GetMediaRequestContextForRenderProcess(int renderer_child_id) {
  content::RenderProcessHost* host =
      content::RenderProcessHost::FromID(renderer_child_id);

  return host->GetStoragePartition()->GetMediaURLRequestContext();
}

net::URLRequestContextGetter*
BrowserContext::GetMediaRequestContextForStoragePartition(
    const base::FilePath& partition_path,
    bool in_memory) {
  // We don't return any storage partition names from
  // ContentBrowserClient::GetStoragePartitionConfigForSite(), so it's a
  // bug to hit this
  NOTREACHED() << "Invalid request for request context for storage partition";
  return NULL;
}

void BrowserContext::RequestMIDISysExPermission(
    int render_process_id,
    int render_view_id,
    const GURL& requesting_frame,
    const MIDISysExPermissionCallback& callback) {
  callback.Run(false);
}

content::ResourceContext* BrowserContext::GetResourceContext() {
  return resource_context_;
}

content::DownloadManagerDelegate*
    BrowserContext::GetDownloadManagerDelegate() {
  return NULL;
}

content::GeolocationPermissionContext*
    BrowserContext::GetGeolocationPermissionContext() {
  return NULL;
}

quota::SpecialStoragePolicy* BrowserContext::GetSpecialStoragePolicy() {
  return NULL;
}

// static
BrowserContext* BrowserContext::GetInstance() {
  return g_browser_context;
}

// static
BrowserContext* BrowserContext::Create() {
  DCHECK(!g_browser_context) <<
      "Shouldn't create more than one master browser context";
  if (GlobalSettings::GetDataPath().empty()) {
    g_browser_context = new OffTheRecordBrowserContextImpl();
  } else {
    g_browser_context = new BrowserContextImpl();
  }

  return g_browser_context;
}

} // namespace oxide
