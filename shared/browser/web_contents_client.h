// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_WEB_CONTENTS_CLIENT_H_
#define _OXIDE_SHARED_BROWSER_WEB_CONTENTS_CLIENT_H_

#include <memory>
#include <string>

#include "base/strings/string16.h"
#include "ui/base/window_open_disposition.h"

#include "shared/browser/web_contents_unique_ptr.h"
#include "shared/common/oxide_shared_export.h"

class GURL;

namespace content {
struct ContextMenuParams;
}

namespace gfx {
class Rect;
}

namespace oxide {

class ResourceDispatcherHostLoginDelegate;
class WebContextMenu;
class WebContextMenuClient;

class OXIDE_SHARED_EXPORT WebContentsClient {
 public:
  virtual ~WebContentsClient();

  virtual bool ShouldHandleNavigation(const GURL& url,
                                      bool user_gesture);

  virtual bool CanCreateWindows();

  virtual bool ShouldCreateNewWebContents(const GURL& url,
                                          WindowOpenDisposition disposition,
                                          bool user_gesture);

  virtual bool AdoptNewWebContents(const gfx::Rect& initial_pos,
                                   WindowOpenDisposition disposition,
                                   WebContentsUniquePtr contents);

  virtual void DownloadRequested(const GURL& url,
                                 const std::string& mime_type,
                                 const bool should_prompt,
                                 const base::string16& suggested_filename,
                                 const std::string& cookies,
                                 const std::string& referrer,
                                 const std::string& user_agent);

  virtual void HttpAuthenticationRequested(
      ResourceDispatcherHostLoginDelegate* login_delegate);

  virtual std::unique_ptr<WebContextMenu> CreateContextMenu(
      const content::ContextMenuParams& params,
      WebContextMenuClient* client);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_CONTENTS_CLIENT_H_
