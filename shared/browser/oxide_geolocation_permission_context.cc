// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include "oxide_geolocation_permission_context.h"

#include "base/bind.h"
#include "base/callback.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "url/gurl.h"

#include "oxide_permission_request.h"
#include "oxide_web_view.h"

namespace oxide {

GeolocationPermissionContext::GeolocationPermissionContext() {}

void GeolocationPermissionContext::RequestGeolocationPermission(
    content::WebContents* contents,
    int bridge_id,
    const GURL& requesting_frame,
    bool user_gesture,
    base::Callback<void(bool)> callback) {
  WebView* webview = WebView::FromWebContents(contents);
  if (!webview) {
    callback.Run(false);
    return;
  }

  int render_process_id = contents->GetRenderProcessHost()->GetID();
  int render_view_id = contents->GetRenderViewHost()->GetRoutingID();
  PermissionRequest::ID id(render_process_id, render_view_id, bridge_id);

  webview->RequestGeolocationPermission(id, requesting_frame.GetOrigin(),
                                        callback);
}

void GeolocationPermissionContext::CancelGeolocationPermissionRequest(
    content::WebContents* contents,
    int bridge_id,
    const GURL& requesting_frame) {
  WebView* webview = WebView::FromWebContents(contents);
  if (!webview) {
    return;
  }

  int render_process_id = contents->GetRenderProcessHost()->GetID();
  int render_view_id = contents->GetRenderViewHost()->GetRoutingID();
  PermissionRequest::ID id(render_process_id, render_view_id, bridge_id);

  webview->CancelGeolocationPermissionRequest(id);
}

} // namespace oxide
