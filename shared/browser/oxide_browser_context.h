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

#ifndef _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_H_
#define _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/content_browser_client.h"

namespace oxide {

class ResourceContext;
class URLRequestContextGetter;

class BrowserContext : public content::BrowserContext {
 public:
  virtual ~BrowserContext();

  static BrowserContext* FromContentBrowserContext(
      content::BrowserContext* context) {
    return static_cast<BrowserContext *>(context);
  }

  static BrowserContext* GetInstance();

  static BrowserContext* Create();

  net::URLRequestContextGetter* CreateRequestContext(
      content::ProtocolHandlerMap* protocol_handlers);

  virtual BrowserContext* GetOffTheRecordContext() = 0;

  virtual BrowserContext* GetOriginalContext() = 0;

  virtual net::URLRequestContextGetter* GetRequestContext() OVERRIDE;

  virtual net::URLRequestContextGetter* GetRequestContextForRenderProcess(
      int renderer_child_id) OVERRIDE;

  virtual net::URLRequestContextGetter* GetMediaRequestContext() OVERRIDE;

  virtual net::URLRequestContextGetter* GetMediaRequestContextForRenderProcess(
      int renderer_child_id) OVERRIDE;

  virtual net::URLRequestContextGetter*
      GetMediaRequestContextForStoragePartition(
          const base::FilePath& partition_path,
          bool in_memory) OVERRIDE;

  virtual void RequestMIDISysExPermission(
      int render_process_id,
      int render_view_id,
      const GURL& requesting_frame,
      const MIDISysExPermissionCallback& callback) OVERRIDE;

  virtual content::ResourceContext* GetResourceContext() OVERRIDE;

  virtual content::DownloadManagerDelegate*
      GetDownloadManagerDelegate() OVERRIDE;

  virtual content::GeolocationPermissionContext*
      GetGeolocationPermissionContext() OVERRIDE;

  virtual quota::SpecialStoragePolicy* GetSpecialStoragePolicy() OVERRIDE;

 protected:
  BrowserContext();

 private:
  scoped_refptr<URLRequestContextGetter> main_request_context_getter_;
  ResourceContext* resource_context_;

  DISALLOW_COPY_AND_ASSIGN(BrowserContext);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_H_
