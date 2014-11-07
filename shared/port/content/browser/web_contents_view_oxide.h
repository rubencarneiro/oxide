// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2014 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_PORT_CONTENT_BROWSER_WEB_CONTENTS_VIEW_H_
#define _OXIDE_SHARED_PORT_CONTENT_BROWSER_WEB_CONTENTS_VIEW_H_

#include "content/browser/renderer_host/render_view_host_delegate_view.h"
#include "content/browser/web_contents/web_contents_view.h"
#include "content/common/content_export.h"

namespace content {

class WebContents;

class WebContentsViewOxide : public WebContentsView,
                             public RenderViewHostDelegateView {
 public:
  virtual ~WebContentsViewOxide() {}
};

typedef WebContentsViewOxide* (WebContentsViewOxideFactory)(WebContents*);

CONTENT_EXPORT void SetWebContentsViewOxideFactory(
    WebContentsViewOxideFactory* factory);

} // namespace content

#endif // _OXIDE_SHARED_PORT_CONTENT_BROWSER_WEB_CONTENTS_VIEW_H_